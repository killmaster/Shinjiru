#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDesktopServices>
#include <QIcon>
#include <QFileDialog>

#include "../app.h"
#include "../api/api.h"
#include "animepanel.h"
#include "fvupdater.h"
#include "about.h"
#include "overlay.h"
#include "searchpanel.h"

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
  download_rule        = 0;
  download_count       = 0;
  hasUser              = false;

  uptime_timer->start();
  watch_timer->setSingleShot(true);

  awesome->initFontAwesome();
  loadSettings();

  QFont font = ui->listTabs->tabBar()->font();
  font.setCapitalization(QFont::Capitalize);
  ui->listTabs->tabBar()->setFont(font);
  ui->orderListWidget->setFont(font);

  int currentYear = QDate::currentDate().year();
  int lowerYear = currentYear - 10;

  while(currentYear > lowerYear) {
    ui->comboYear->addItem(QString::number(currentYear));
    currentYear--;
  }

  ui->browseTabs->tabBar()->setVisible(false);
  ui->browseTabs->setCurrentIndex(0);

  QWidget *container = new QWidget(ui->scrollArea);
  layout = new FlowLayout(container);
  ui->scrollArea->setWidget(container);
  container->setLayout(layout);

  QWidget *container2 = new QWidget(ui->scrollArea_2);
  layout2 = new FlowLayout(container2);
  ui->scrollArea_2->setWidget(container2);
  container2->setLayout(layout2);

  QVariantMap black;
  black.insert("color", QColor(0, 0, 0));
  black.insert("color-active", QColor(0, 0, 0));
  black.insert("color-disabled", QColor(0, 0, 0));
  black.insert("color-selected", QColor(0, 0, 0));

  ui->airingButton    ->setIcon(awesome->icon(fa::clocko,    black));
  ui->torrentsButton  ->setIcon(awesome->icon(fa::rss,       black));
  ui->animeButton     ->setIcon(awesome->icon(fa::bars,      black));
  ui->seasonsButton   ->setIcon(awesome->icon(fa::th,        black));
  ui->statisticsButton->setIcon(awesome->icon(fa::piechart,  black));
  ui->moveDownButton  ->setIcon(awesome->icon(fa::arrowdown, black));
  ui->moveUpButton    ->setIcon(awesome->icon(fa::arrowup,   black));

  ui->moveUpButton->setText("");
  ui->moveDownButton->setText("");

  ui->tabWidget->tabBar()->hide();
  ui->tabWidget->setCurrentIndex(0);
  ui->listTabs->setCurrentIndex(0);

  ui->statusBar->addWidget(progress_bar);
  ui->statusBar->layout()->setContentsMargins(1,0,0,0);
  progress_bar->setRange(0, 100);
  progress_bar->setValue(5);
  progress_bar->setFormat("Authorizing");

  QSettings s;
  restoreGeometry(s.value("mainWindowGeometry").toByteArray());
  restoreState(s.value("mainWindowState").toByteArray());

  connect(&user_future_watcher,      SIGNAL(finished()), SLOT(userLoaded()));
  connect(&user_list_future_watcher, SIGNAL(finished()), SLOT(userListLoaded()));

  connect(ui->animeButton,      SIGNAL(clicked()),   SLOT(showAnimeTab()));
  connect(ui->airingButton,     SIGNAL(clicked()),   SLOT(showAiringTab()));
  connect(ui->torrentsButton,   SIGNAL(clicked()),   SLOT(showTorrentsTab()));
  connect(ui->seasonsButton,    SIGNAL(clicked()),   SLOT(showBrowseTab()));
  connect(ui->actionSettings,   SIGNAL(triggered()), SLOT(showSettingsTab()));
  connect(ui->statisticsButton, SIGNAL(clicked()),   SLOT(showStatisticsTab()));

  connect(ui->actionExit,   &QAction::triggered,                             [&]() {
    QSettings s;
    s.setValue("mainWindowGeometry", saveGeometry());
    s.setValue("mainWindowState", saveState());

    qApp->quit();
  });
  connect(ui->actionAbout,  &QAction::triggered,                             [&]() {About *about = new About(this); about->show();});
  connect(ui->actionHelp,   &QAction::triggered,                             [&]() {QDesktopServices::openUrl(QUrl("http://app.shinjiru.me/support.php"));});
  connect(ui->actionRL,     &QAction::triggered,                             [&]() {loadUser();});
  connect(ui->actionAS,     &QAction::triggered,                             [&]() {SearchPanel *sp = new SearchPanel(this); sp->show();});
  connect(ui->actionVAL,    SIGNAL(triggered()),                             SLOT(viewAnimeList()));
  connect(ui->actionVD,     SIGNAL(triggered()),                             SLOT(viewDashboard()));
  connect(ui->actionVP,     SIGNAL(triggered()),                             SLOT(viewProfile()));
  connect(ui->actionEAR,    SIGNAL(triggered(bool)),                         SLOT(toggleAnimeRecognition(bool)));
  connect(ui->actionUpdate, SIGNAL(triggered()), FvUpdater::sharedUpdater(), SLOT(CheckForUpdatesNotSilent()));
  connect(ui->actionExport, SIGNAL(triggered()),                             SLOT(exportListJSON()));

  connect(ui->disconnectButton, SIGNAL(clicked()), SLOT(resetAPI()));

  connect(window_watcher, SIGNAL(title_found(QString)), SLOT(watch(QString)));
  connect(watch_timer,    SIGNAL(timeout()),            SLOT(updateEpisode()));
  connect(event_timer,    SIGNAL(timeout()),            SLOT(eventTick()));

  connect(ui->comboSeason, SIGNAL(currentTextChanged(QString)), SLOT(loadSelectedSeason()));
  connect(ui->comboYear, SIGNAL(currentTextChanged(QString)), SLOT(loadSelectedSeason()));

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
  connect(ui->updateStreamComboBox,           SIGNAL(currentTextChanged(QString)), SLOT(settingsChanged()));
  connect(ui->minimizeToTrayCheckBox,         SIGNAL(toggled(bool)),               SLOT(settingsChanged()));
  connect(ui->closeToTrayCheckBox,            SIGNAL(toggled(bool)),               SLOT(settingsChanged()));
  connect(ui->applyButton,                    SIGNAL(clicked()),                   SLOT(applySettings()));
  connect(ui->defaultButton,                  SIGNAL(clicked()),                   SLOT(defaultSettings()));
  connect(ui->moveUpButton,                   SIGNAL(clicked()),                   SLOT(moveUp()));
  connect(ui->moveDownButton,                 SIGNAL(clicked()),                   SLOT(moveDown()));
  connect(ui->openSkinsFolderButton,          &QPushButton::clicked,               [&]() {
    QDesktopServices::openUrl(QUrl(qApp->applicationDirPath() + "/data/skin/"));
  });

  connect(ui->tabWidget, &QTabWidget::currentChanged, [&](int tab) {
    if(tab != 0) {
      this->over->removeDrawing("blank_table");
    } else {
      this->filterList(3);
    }
  });

  connect(ui->listFilterLineEdit, SIGNAL(textChanged(QString)), SLOT(filterList(QString)));
  connect(ui->listFilterLineEdit, SIGNAL(returnPressed()), SLOT(showSearch()));
  connect(ui->listTabs, SIGNAL(currentChanged(int)), SLOT(filterList(int)));

  connect(ui->airingButton_2,  SIGNAL(clicked()), SLOT(loadAiringBrowser()));
  connect(ui->seasonsButton_2, SIGNAL(clicked()), SLOT(loadSeasonBrowser()));
  connect(ui->upcomingButton,  SIGNAL(clicked()), SLOT(loadUpcomingBrowser()));
  connect(ui->recentButton,    SIGNAL(clicked()), SLOT(loadRecentBrowser()));
  connect(ui->browseButton,    SIGNAL(clicked()), SLOT(loadBrowserData()));

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

      event_timer->start(1000);
    });

    connect(API::sharedAPI()->sharedAniListAPI(), &AniListAPI::access_denied, [&](QString error) {
      error = error.replace(api_id, "<API ID>");
      error = error.replace(api_secret, "<API SECRET>");

      qCritical() << error;

      QMessageBox::critical(this, "Shinjiru", tr("Error: ") + error);
    });
  }

  reloadRules();
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

void MainWindow::mouseDoubleClickEvent(QMouseEvent *event) {
  if(over->containsDrawing("blank_table")) {
    QTableWidget *w = static_cast<QTableWidget *>(ui->listTabs->currentWidget()->layout()->itemAt(0)->widget());
    QPoint location = w->mapTo(this, QPoint(0,0));
    QRect r(location.x()+1, location.y()+24, w->width()-2, w->height()-25);

    if(r.contains(event->x(), event->y())) {
      SearchPanel *sp = new SearchPanel(this);
      sp->show();
    }
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


  width = layout2->geometry().width();
  cwidth = layout2->contentsWidth();

  layout2->setContentsMargins((width-cwidth)/2, 0, 0, 0);
}

void MainWindow::paintEvent(QPaintEvent *event) {
  QPainter p(this);

  p.setRenderHint(QPainter::SmoothPixmapTransform, true);


  int notification_count = 0;

  if(this->hasUser) {
    p.drawPixmap(width() - 52, 24, 42, 42, User::sharedUser()->userImage());
    notification_count = User::sharedUser()->notificationCount();
  }

  p.drawRect(width() - 52, 24, 42, 42);

  if(notification_count > 0) p.setBrush(QColor(255,120,120));
  else p.setBrush(QColor(255,255,255));

  p.drawEllipse(width() - 20.0f, 57.0f, 17.0f, 17.0f);
  QString notif = notification_count >= 9 ? "9+" : QString::number(notification_count);
  p.drawText(width() - 19.0f, 57.0f, 17.0f, 17.0f, Qt::AlignCenter, notif);

  QFont font = p.font();
  font.setPointSize(12);
  p.setFont(font);

  if(this->hasUser) {
    p.drawText(0, 25, width() - 57, 40, Qt::AlignRight, User::sharedUser()->displayName());
  }

  event->accept();
}

void MainWindow::showAnimeTab()      { ui->tabWidget->setCurrentIndex(0); }
void MainWindow::showSettingsTab()   { ui->tabWidget->setCurrentIndex(1); }
void MainWindow::showTorrentsTab()   { ui->tabWidget->setCurrentIndex(2); }
void MainWindow::showAiringTab()     { ui->tabWidget->setCurrentIndex(3);
                                       int width = layout->geometry().width();
                                       int cwidth = layout->contentsWidth();

                                       if(cwidth < 0) {
                                         width = this->width() - 2;
                                         cwidth = this->width() - (this->width() % 200);
                                       }
                                       layout->setContentsMargins((width-cwidth)/2, 0, 0, 0); }
void MainWindow::showStatisticsTab() { ui->tabWidget->setCurrentIndex(4); }
void MainWindow::showBrowseTab()     { ui->tabWidget->setCurrentIndex(5);
                                    int width = layout2->geometry().width();
                                    int cwidth = layout2->contentsWidth();

                                    if(cwidth < 0) {
                                      width = this->width() - 2;
                                      cwidth = this->width() - (this->width() % 200);
                                    }

                                    layout2->setContentsMargins((width-cwidth)/2, 0, 0, 0); }

void MainWindow::showAnimePanel(int row, int column, QTableWidget *source) {
  Q_UNUSED(column);

  QString title = source->item(row, 0)->text();
  QString episodes = source->item(row, 1)->text();
  QString score = source->item(row, 2)->text();
  QString type = source->item(row, source->columnCount() - 1)->text();
  Anime *anime = User::sharedUser()->getAnimeByData(title, episodes, score, type);
  QString old_status = anime->getMyStatus();

  if(anime == nullptr) {
    QMessageBox::critical(this, "Shinjiru", tr("Unknown anime."));
    return;
  }


  AnimePanel *ap = new AnimePanel(this, anime, User::sharedUser()->scoreType());

  if(anime->needsLoad() || anime->needsCover()) {
    User::sharedUser()->loadAnimeData(anime, true);
  }

  connect(ap, &AnimePanel::accepted, [&, source, anime, row, old_status]() {
    this->userListLoaded();
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

  if(cw_anime->getMyProgress() == cw_anime->getEpisodeCount() && cw_anime->getEpisodeCount() != 0) {
    data.insert("list_status", "completed");
    cw_anime->setMyStatus("completed");
  }

  userListLoaded();

  QJsonObject response = API::sharedAPI()->sharedAniListAPI()->put(API::sharedAPI()->sharedAniListAPI()->API_EDIT_LIST, data).object();

  if(response != QJsonObject()) {
    trayIcon->showMessage("Shinjiru", tr("%1 updated to episode %2").arg(cw_anime->getTitle()).arg(cw_episode));
    if(data.value("list_status") == "completed") {
      trayIcon->showMessage("Shinjiru", tr("%1 marked as completed").arg(cw_anime->getTitle()));
    }
  }
  else {
    trayIcon->showMessage("Shinjiru", tr("%1 was not successfully updated").arg(cw_anime->getTitle()));
  }
}

void MainWindow::eventTick() {
  if(torrent_refresh_time == 0) {
    torrent_refresh_time = torrent_interval;
    refreshTorrentListing();
  }

  if(user_refresh_time == 0) {
    user_refresh_time = 15 * 60 * 1000;
    loadUser();
  }

  for(int i = 0; i < airing_anime.count(); i++) {
      AiringAnime *aa = airing_anime.at(i);

      if(!aa->getAnime()->needsLoad())
        aa->tick();
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

void MainWindow::exportListJSON() {
  if(!this->hasUser) return;

  QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), qApp->applicationDirPath(),tr("Json File (*.json)"));

  if(fileName.isEmpty()) return;
  QFile f(fileName);
  f.open(QFile::WriteOnly);
  if(f.isOpen()) {
    QByteArray data = User::sharedUser()->listJson();
    int bytes = f.write(data);

    if(bytes != data.length() || bytes == -1) {
      QMessageBox::information(this, "Shinjiru", tr("Error %d: File could not be written to").arg(bytes));
    }

    f.close();
  }
}


void MainWindow::showSearch() {
  SearchPanel *sp = new SearchPanel(this);
  sp->setSearch(ui->listFilterLineEdit->text());
  sp->show();
}
