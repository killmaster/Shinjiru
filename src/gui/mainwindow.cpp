#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDesktopServices>
#include <QIcon>

#include "../app.h"
#include "../api/api.h"
#include "animepanel.h"
#include "fvupdater.h"
#include "about.h"
#include "overlay.h"

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  QFile styleFile(":/style.css");
  styleFile.open(QFile::ReadOnly);
  QString styleSheet = QLatin1String(styleFile.readAll());
  qApp->setStyleSheet(styleSheet);

  QPixmap window_icon;
  QFile iconFile(":/icon.png");
  iconFile.open(QFile::ReadOnly);
  QByteArray icon_data = iconFile.readAll();
  window_icon.loadFromData(icon_data);
  qApp->setWindowIcon(QIcon(window_icon));

  awesome              = new QtAwesome(qApp);
  settings             = new Settings(0);
  window_watcher       = new WindowWatcher(0);
  anitomy              = new AnitomyWrapper();
  event_timer          = new QTimer(this);
  watch_timer          = new QTimer(this);
  uptime_timer         = new QElapsedTimer;
  progress_bar         = new QProgressBar(ui->statusBar);
  over                 = new Overlay(this);
  torrent_refresh_time = 0;
  user_refresh_time    = 15 * 60 * 1000;
  list_refresh_time    = 60 * 60 * 1000;
  download_rule        = 0;
  download_count       = 0;
  hasUser              = false;
  needsRefresh         = true;

  uptime_timer->start();
  watch_timer->setSingleShot(true);

  awesome->initFontAwesome();
  loadSettings();

  QFont font = ui->listTabs->tabBar()->font();
  font.setCapitalization(QFont::Capitalize);
  ui->listTabs->tabBar()->setFont(font);
  ui->orderListWidget->setFont(font);


  QWidget *container = new QWidget(ui->scrollArea);
  layout = new FlowLayout(container);
  ui->scrollArea->setWidget(container);
  container->setLayout(layout);

  QVariantMap black;
  black.insert("color", QColor(0, 0, 0));
  black.insert("color-active", QColor(0, 0, 0));
  black.insert("color-disabled", QColor(0, 0, 0));
  black.insert("color-selected", QColor(0, 0, 0));

  ui->airingButton    ->setIcon(awesome->icon(fa::clocko,    black));
  ui->torrentsButton  ->setIcon(awesome->icon(fa::rss,       black));
  ui->animeButton     ->setIcon(awesome->icon(fa::bars,      black));
  ui->statisticsButton->setIcon(awesome->icon(fa::piechart,  black));
  ui->moveDownButton  ->setIcon(awesome->icon(fa::arrowdown, black));
  ui->moveUpButton    ->setIcon(awesome->icon(fa::arrowup,   black));

  ui->moveUpButton->setText("");
  ui->moveDownButton->setText("");

  ui->tabWidget->tabBar()->hide();
  ui->tabWidget->setCurrentIndex(0);
  ui->listTabs->setCurrentIndex(0);

  ui->statusBar->addWidget(progress_bar);
  progress_bar->setRange(0, 100);
  progress_bar->setValue(5);
  progress_bar->setFormat("Authorizing");

  connect(&user_future_watcher,      SIGNAL(finished()), SLOT(userLoaded()));
  connect(&user_list_future_watcher, SIGNAL(finished()), SLOT(userListLoaded()));

  connect(ui->animeButton,      SIGNAL(clicked()),   SLOT(showAnimeTab()));
  connect(ui->airingButton,     SIGNAL(clicked()),   SLOT(showAiringTab()));
  connect(ui->torrentsButton,   SIGNAL(clicked()),   SLOT(showTorrentsTab()));
  connect(ui->actionSettings,   SIGNAL(triggered()), SLOT(showSettingsTab()));
  connect(ui->statisticsButton, SIGNAL(clicked()),   SLOT(showStatisticsTab()));

  connect(ui->actionExit,   &QAction::triggered,                             []()  {exit(0);});
  connect(ui->actionAbout,  &QAction::triggered,                             [&]() {showAbout();});
  connect(ui->actionHelp,   &QAction::triggered,                             [&]() {QDesktopServices::openUrl(QUrl("http://app.shinjiru.me/support.php"));});
  connect(ui->actionRL,     &QAction::triggered,                             [&]() {needsRefresh = true; loadUserList();});
  connect(ui->actionVAL,    SIGNAL(triggered()),                             SLOT(viewAnimeList()));
  connect(ui->actionVD,     SIGNAL(triggered()),                             SLOT(viewDashboard()));
  connect(ui->actionVP,     SIGNAL(triggered()),                             SLOT(viewProfile()));
  connect(ui->actionEAR,    SIGNAL(triggered(bool)),                         SLOT(toggleAnimeRecognition(bool)));
  connect(ui->actionUpdate, SIGNAL(triggered()), FvUpdater::sharedUpdater(), SLOT(CheckForUpdatesNotSilent()));

  connect(ui->disconnectButton, SIGNAL(clicked()), SLOT(resetAPI()));

  connect(window_watcher, SIGNAL(title_found(QString)), SLOT(watch(QString)));
  connect(watch_timer,    SIGNAL(timeout()),            SLOT(updateEpisode()));
  connect(event_timer,    SIGNAL(timeout()),            SLOT(eventTick()));

  connect(ui->torrentTable,      SIGNAL(customContextMenuRequested(QPoint)), SLOT(torrentContextMenu(QPoint)));
  connect(ui->torrentFilter,     SIGNAL(textChanged(QString)),               SLOT(filterTorrents(QString)));
  connect(ui->chkHideUnknown,    SIGNAL(toggled(bool)),                      SLOT(filterTorrents(bool)));
  connect(ui->refreshButton,     SIGNAL(clicked()),                          SLOT(refreshTorrentListing()));
  connect(ui->ruleManagerButton, SIGNAL(clicked()),                          SLOT(showRuleManager()));

  connect(ui->actionEAR,                      SIGNAL(toggled(bool)),               SLOT(applyEAR()));
  connect(ui->torrentRefreshIntervalLineEdit, SIGNAL(textEdited(QString)),         SLOT(settingsChanged()));
  connect(ui->autoRecognitionCheckBox,        SIGNAL(toggled(bool)),               SLOT(settingsChanged()));
  connect(ui->defaultTorrentRuleModeComboBox, SIGNAL(currentTextChanged(QString)), SLOT(settingsChanged()));
  connect(ui->startOnBootCheckBox,            SIGNAL(toggled(bool)),               SLOT(settingsChanged()));
  connect(ui->minimizeToTrayCheckBox,         SIGNAL(toggled(bool)),               SLOT(settingsChanged()));
  connect(ui->closeToTrayCheckBox,            SIGNAL(toggled(bool)),               SLOT(settingsChanged()));
  connect(ui->applyButton,                    SIGNAL(clicked()),                   SLOT(applySettings()));
  connect(ui->defaultButton,                  SIGNAL(clicked()),                   SLOT(defaultSettings()));
  connect(ui->moveUpButton,                   SIGNAL(clicked()),                   SLOT(moveUp()));
  connect(ui->moveDownButton,                 SIGNAL(clicked()),                   SLOT(moveDown()));

  connect(ui->tabWidget, &QTabWidget::currentChanged, [&](int tab) {
    if(tab != 0) {
      this->over->removeDrawing("blank_table");
    } else {
      this->filterList(3);
    }
  });

  connect(ui->listFilterLineEdit, SIGNAL(textChanged(QString)), SLOT(filterList(QString)));
  connect(ui->listTabs, SIGNAL(currentChanged(int)), SLOT(filterList(int)));

  this->show();
  createActions();
  initTray();
  trayIcon->show();

  int result = API::sharedAPI()->verify();

  if(result == AniListAPI::OK) {
    connect(API::sharedAPI()->sharedAniListAPI(), &AniListAPI::access_granted, [&]() {
      progress_bar->setValue(10);
      progress_bar->setFormat("Access granted");
      loadUser();
      reloadRules();

      event_timer->start(1000);
    });

    connect(API::sharedAPI()->sharedAniListAPI(), &AniListAPI::access_denied, [&](QString error) {
      error = error.replace(api_id, "<API ID>");
      error = error.replace(api_secret, "<API SECRET>");

      qCritical() << error;

      QMessageBox::critical(this, "Shinjiru", tr("Error: ") + error);
    });
  }
}

MainWindow::~MainWindow() {
  trayIcon->hide();

  delete trayIcon;
  delete ui;
  delete awesome;
  delete anitomy;
  delete settings;
  delete window_watcher;
}

void MainWindow::closeEvent(QCloseEvent *event) {
  if(trayIcon->isVisible() && close_to_tray) {
    hide();
    event->ignore();
  }
}

void MainWindow::changeEvent(QEvent *event) {
  if(event->type() == QEvent::WindowStateChange) {
    if(isMaximized()) {
      showFunc = "showMaximized";
    } else  if(isMinimized() && minimize_to_tray) {
      this->hide();
      event->ignore();
    } else {
      showFunc = "showNormal";
    }
  }
}

void MainWindow::resizeEvent(QResizeEvent *event) {
  over->resize(event->size());

  if(over->containsDrawing("blank_table"))
    this->addSearchPrompt();

  event->accept();
  int width = layout->geometry().width();
  int cwidth = layout->contentsWidth();

  layout->setContentsMargins((width-cwidth)/2, 0, 0, 0);
}

void MainWindow::paintEvent(QPaintEvent *event) {
  QPainter p(this);

  QFont font = p.font();
  font.setPointSize(14);
  p.setFont(font);

  p.setRenderHint(QPainter::SmoothPixmapTransform, true);

  if(hasUser) {
    p.drawText(0, 30, width() - 65, 40, Qt::AlignRight, User::sharedUser()->displayName());
    p.drawPixmap(width() - 60, 24, 50, 50, User::sharedUser()->userImage());
  }

  p.drawRect(width() - 60, 24, 50, 50);

  event->accept();
}

void MainWindow::showAnimeTab()      { ui->tabWidget->setCurrentIndex(0); }
void MainWindow::showSettingsTab()   { ui->tabWidget->setCurrentIndex(1); }
void MainWindow::showTorrentsTab()   { ui->tabWidget->setCurrentIndex(2); }
void MainWindow::showAiringTab()     { ui->tabWidget->setCurrentIndex(3);
                                       int width = layout->geometry().width();
                                       int cwidth = layout->contentsWidth();

                                       if(cwidth == -6) {
                                         width = this->width() - 2;
                                         cwidth = 747;
                                       }
                                       layout->setContentsMargins((width-cwidth)/2, 0, 0, 0); }
void MainWindow::showStatisticsTab() { ui->tabWidget->setCurrentIndex(4); }

void MainWindow::showAnimePanel(int row, int column, QTableWidget *source) {
  Q_UNUSED(column);

  QString title = source->item(row, 0)->text();
  QString episodes = source->item(row, 1)->text();
  QString score = source->item(row, 2)->text();
  QString type = source->item(row, 3)->text();
  Anime *anime = User::sharedUser()->getAnimeByData(title, episodes, score, type);
  QString old_status = anime->getMyStatus();

  if(anime == 0) {
    QMessageBox::critical(this, "Shinjiru", "Unknown anime.");
    return;
  }

  AnimePanel *ap = new AnimePanel(this, anime, User::sharedUser()->scoreType());

  if(anime->needsLoad() || anime->needsCover()) {
    User::sharedUser()->loadAnimeData(anime, true);
  }

  connect(ap, &AnimePanel::destroyed, [&, source, anime, row, old_status]() {
    static_cast<ProgressTableWidgetItem *>(source->item(row, 1))->setText(QString::number(anime->getMyProgress()) + " / " + QString::number(anime->getEpisodeCount()));
    QTableWidgetItem *scoreData = source->item(row, 2);
    if(User::sharedUser()->scoreType() == 0 || User::sharedUser()->scoreType() == 1) {
      scoreData->setData(Qt::DisplayRole, anime->getMyScore().toInt());
    } else if(User::sharedUser()->scoreType() == 4) {
      scoreData->setData(Qt::DisplayRole, anime->getMyScore().toDouble());
    } else {
      scoreData->setText(anime->getMyScore());
    }

    if(anime->getMyStatus() != old_status) {
      int tab = -1;
      int old_tab = -1;
      for(int t = 0; t < ui->listTabs->count(); t++) {
        if(ui->listTabs->tabText(t).contains(anime->getMyStatus())) {
          tab = t;
        }
        if(ui->listTabs->tabText(t).contains(old_status)) {
          old_tab = t;
        }
      }

      if(tab == -1) return;
      ui->listTabs->setCurrentIndex(tab);
      QTableWidget *w = static_cast<QTableWidget *>(ui->listTabs->widget(tab)->layout()->itemAt(0)->widget());
      w->insertRow(0);
      w->setItem(0, 0, source->takeItem(row, 0));
      w->setItem(0, 1, source->takeItem(row, 1));
      w->setItem(0, 2, source->takeItem(row, 2));
      w->setItem(0, 3, source->takeItem(row, 3));
      source->removeRow(row);
      ui->listTabs->setTabText(tab, anime->getMyStatus() + "(" + QString::number(w->rowCount()) + ")");
      ui->listTabs->setTabText(old_tab, old_status + "(" + QString::number(source->rowCount()) + ")");
    }

    source->clearSelection();
    updateStatistics();
  });

  ap->show();
}

void MainWindow::showAnimePanel(int row, int column) {
  Q_UNUSED(column);

  QTableWidget *source = static_cast<QTableWidget *>(sender());

  showAnimePanel(row, column, source);
}

void MainWindow::viewDashboard() { QDesktopServices::openUrl(QString("http://anilist.co/home")); }
void MainWindow::viewProfile()   { QDesktopServices::openUrl(QString("http://anilist.co/user/") + User::sharedUser()->displayName()); }
void MainWindow::viewAnimeList() { QDesktopServices::openUrl(QString("http://anilist.co/animelist/") + User::sharedUser()->displayName()); }

void MainWindow::toggleAnimeRecognition(bool checked) {
  if(checked) {
    window_watcher->enable();
  } else {
    window_watcher->disable();
  }

  ui->actionEAR->setChecked(checked);
  ui->autoRecognitionCheckBox->setChecked(checked);
}

void MainWindow::watch(QString title) {
  if(this->currently_watching != title) {
    this->currently_watching = title;
    this->watch_timer->stop();

    QMap<QString, QString> results = anitomy->parse(title);

    this->cw_title   = results.value("title");
    this->cw_episode = results.value("episode");
    cw_episode = cw_episode.remove(QRegExp("^[0]*"));

    if(cw_episode.isEmpty() || cw_title.isEmpty()) {
        return;
    }

    cw_anime = User::sharedUser()->getAnimeByTitle(cw_title);

    if(cw_anime == 0 || cw_anime->getTitle().isEmpty()) {
      QJsonObject results = API::sharedAPI()->sharedAniListAPI()->get(API::sharedAPI()->sharedAniListAPI()->API_ANIME_SEARCH(cw_title)).array().at(0).toObject();
      cw_anime = User::sharedUser()->getAnimeByTitle(results.value("title_romaji").toString());
      if(cw_anime == 0) return;
    }

    if(cw_anime->getMyProgress() >= cw_episode.toInt()) {
      return;
    }

    if(cw_anime->getMyStatus() != "watching" && cw_anime->getMyStatus() != "plan to watch") {
      return;
    }

    this->watch_timer->start(1000 * auto_update_delay);
    this->trayIcon->showMessage("Shinjiru", tr("Updating %1 to episode %2 in %3 seconds").arg(cw_anime->getTitle()).arg(cw_episode).arg(QString::number(auto_update_delay)));
  }
}

void MainWindow::updateEpisode() {
  QMap<QString, QString> data;
  data.insert("id",               cw_anime->getID());
  data.insert("episodes_watched", cw_episode);
  data.insert("list_status",      "watching");

  cw_anime->setMyProgress(cw_episode.toInt());
  userListLoaded();

  QJsonObject response = API::sharedAPI()->sharedAniListAPI()->put(API::sharedAPI()->sharedAniListAPI()->API_EDIT_LIST, data).object();

  if(response != QJsonObject())
    trayIcon->showMessage("Shinjiru", cw_anime->getTitle() + (" updated to episode ") + cw_episode);
  else
    trayIcon->showMessage("Shinjiru", cw_anime->getTitle() + (" was not successfully updated"));
}

void MainWindow::eventTick() {
  if(torrent_refresh_time == 0) {
    torrent_refresh_time = torrent_interval;
    refreshTorrentListing();
  }

  if(list_refresh_time == 0) {
    this->needsRefresh = true;
    list_refresh_time = 60 * 0 * 1000;
  }

  if(user_refresh_time == 0) {
    user_refresh_time = 15 * 60 * 1000;
    loadUser();
  }

  torrent_refresh_time--;
  user_refresh_time--;
  ui->refreshButton->setText(tr("Refresh (%1)").arg(QString::number(torrent_refresh_time)));

  qint64 seconds = uptime_timer->elapsed() / 1000;
  int minutes = seconds / 60;
  int hours = seconds / 60.0 / 60;
  int days = seconds / 60.0 / 60.0 / 24;

  seconds %= 60;
  minutes %= 60;
  hours %= 24;

  QString format = "";
  QString comma = "";

  if(days > 1) {
    format += QString::number(days) + tr(" days");
  } else if (days > 0) {
    format += QString::number(days) +tr( " day");
  }

  if(days > 0) comma = ", ";

  if(hours > 1) {
    format += comma + QString::number(hours) + tr(" hours");
  } else if (hours > 0) {
    format += comma + QString::number(hours) + tr(" hour");
  }

  if(hours > 0) comma = ", ";

  if(days == 0) {
    if(minutes > 1) {
      format += comma + QString::number(minutes) + tr(" minutes");
    } else if (minutes > 0) {
      format += comma + QString::number(minutes) + tr(" minute");
    }
  }

  if(minutes > 0) comma = ", ";

  if(hours == 0) {
    if(seconds == 1) {
      format += comma + QString::number(seconds) + tr(" second");
    } else {
      format += comma + QString::number(seconds) + tr(" seconds");
    }
  }

  ui->lblUptime->setText(format);
}

void MainWindow::showAbout() {
  About *about = new About(this);
  about->show();
}

void MainWindow::resetAPI() {
  API::sharedAPI()->sharedAniListAPI()->setAuthorizationCode("");
  API::sharedAPI()->sharedAniListAPI()->setAuthorizationPin("");

  settings->setValue(Settings::AniListAccess, "");
  settings->setValue(Settings::AniListExpires, QDateTime::currentDateTimeUtc());
  settings->setValue(Settings::AniListRefresh, "");

  QProcess::startDetached(QApplication::applicationFilePath());
  exit(0);
}

void MainWindow::moveUp() {
 if(ui->orderListWidget->selectedItems().count() == 1) {
   int row = ui->orderListWidget->row(ui->orderListWidget->selectedItems().at(0));
   if(row != 0) {
     ui->orderListWidget->insertItem(row - 1, ui->orderListWidget->takeItem(row)->text());
     ui->orderListWidget->setCurrentRow(row - 1);
     this->settingsChanged();
   }
 }
}

void MainWindow::moveDown() {
  if(ui->orderListWidget->selectedItems().count() == 1) {
    int row = ui->orderListWidget->row(ui->orderListWidget->selectedItems().at(0));
    if(row != ui->orderListWidget->count()) {
      ui->orderListWidget->insertItem(row + 1, ui->orderListWidget->takeItem(row)->text());
      ui->orderListWidget->setCurrentRow(row + 1);
      this->settingsChanged();
    }
  }
}
