/* Copyright 2015 Kazakuri */

#include "./mainwindow.h"

#include <QDesktopServices>
#include <QIcon>
#include <QFileDialog>
#include <QJsonArray>

#include "./ui_mainwindow.h"
#include "../app.h"
#include "../api/api.h"
#include "./animepanel.h"
#include "./fvupdater.h"
#include "./about.h"
#include "./overlay.h"
#include "./searchpanel.h"
#include "./settingsdialog.h"

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent), ui(new Ui::MainWindow) {
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
  default_time         = 15 * 60;
  user_refresh_time    = default_time;
  download_rule        = 0;
  download_count       = 0;
  hasUser              = false;

  uptime_timer->start();
  watch_timer->setSingleShot(true);

  awesome->initFontAwesome();

  QFont font = ui->listTabs->tabBar()->font();
  font.setCapitalization(QFont::Capitalize);
  ui->listTabs->tabBar()->setFont(font);

  int currentYear = QDate::currentDate().year();
  int lowerYear = currentYear - 10;

  while (currentYear > lowerYear) {
    ui->comboYear->addItem(QString::number(currentYear));
    currentYear--;
  }

  QWidget *container = new QWidget(ui->scrollArea);
  layout = new FlowLayout(container);
  ui->scrollArea->setWidget(container);
  container->setLayout(layout);

  QWidget *container2 = new QWidget(ui->scrollArea_2);
  layout2 = new FlowLayout(container2);
  layout2->disableSort();
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

  ui->tabWidget->tabBar()->hide();
  ui->tabWidget->setCurrentIndex(0);
  ui->listTabs->setCurrentIndex(0);

  ui->statusBar->addWidget(progress_bar);
  ui->statusBar->layout()->setContentsMargins(1, 0, 0, 0);
  progress_bar->setRange(0, 100);
  progress_bar->setValue(5);
  progress_bar->setFormat("Authorizing");

  QSettings s;
  restoreGeometry(s.value("mainWindowGeometry").toByteArray());
  restoreState(s.value("mainWindowState").toByteArray());

  connect(&user_future_watcher,      SIGNAL(finished()), SLOT(userLoaded()));
  connect(&user_list_future_watcher, SIGNAL(finished()),
          SLOT(userListLoaded()));

  connect(ui->animeButton,      SIGNAL(clicked()),   SLOT(showAnimeTab()));
  connect(ui->airingButton,     SIGNAL(clicked()),   SLOT(showAiringTab()));
  connect(ui->torrentsButton,   SIGNAL(clicked()),   SLOT(showTorrentsTab()));
  connect(ui->seasonsButton,    SIGNAL(clicked()),   SLOT(showBrowseTab()));
  connect(ui->actionSettings,   SIGNAL(triggered()), SLOT(showSettings()));
  connect(ui->statisticsButton, SIGNAL(clicked()),   SLOT(showStatisticsTab()));

  connect(ui->actionExit, &QAction::triggered, [&]() {  // NOLINT
    this->elegantClose();
  });

  connect(ui->actionAbout, &QAction::triggered, [&]() {  // NOLINT
    About *about = new About(this);
    about->show();
  });

  connect(ui->actionHelp, &QAction::triggered, [&]() {  // NOLINT
    QDesktopServices::openUrl(QUrl("http://app.shinjiru.me/support.php"));
  });

  connect(ui->actionRL, &QAction::triggered, [&]() {  // NOLINT
    loadUser();
  });

  connect(ui->actionAS, &QAction::triggered, [&]() {  // NOLINT
    SearchPanel *sp = new SearchPanel(this);
    sp->show();
  });

  connect(ui->actionARR, &QAction::triggered, [&]() {  // NOLINT
    reloadSmartTitles();
    SettingsDialog *s = new SettingsDialog(this);
    s->showSmartTitles();
  });

  connect(ui->actionVAL, SIGNAL(triggered()), SLOT(viewAnimeList()));
  connect(ui->actionVD, SIGNAL(triggered()), SLOT(viewDashboard()));
  connect(ui->actionVP, SIGNAL(triggered()), SLOT(viewProfile()));
  connect(ui->actionEAR, SIGNAL(triggered(bool)),
          SLOT(toggleAnimeRecognition(bool)));
  connect(ui->actionExport, SIGNAL(triggered()), SLOT(exportListJSON()));
  connect(ui->actionUpdate, SIGNAL(triggered()),
          FvUpdater::sharedUpdater(), SLOT(CheckForUpdatesNotSilent()));

  connect(window_watcher, SIGNAL(title_found(QString)), SLOT(watch(QString)));
  connect(watch_timer, SIGNAL(timeout()), SLOT(updateEpisode()));
  connect(event_timer, SIGNAL(timeout()), SLOT(eventTick()));

  connect(ui->torrentTable, SIGNAL(customContextMenuRequested(QPoint)),
          SLOT(torrentContextMenu(QPoint)));
  connect(ui->torrentFilter, SIGNAL(textChanged(QString)),
          SLOT(filterTorrents(QString)));
  connect(ui->chkHideUnknown, SIGNAL(toggled(bool)),
          SLOT(filterTorrents(bool)));
  connect(ui->refreshButton, SIGNAL(clicked()), SLOT(refreshTorrentListing()));
  connect(ui->ruleManagerButton, SIGNAL(clicked()), SLOT(showRuleManager()));

  connect(ui->actionEAR, SIGNAL(toggled(bool)), SLOT(applyEAR()));

  connect(ui->tabWidget, &QTabWidget::currentChanged, [&](int tab) {  // NOLINT
    if (tab != 0) {
      this->over->removeDrawing("blank_table");
      this->over->removeDrawing("no anime");
      ui->listTabs->show();
      ui->listFilterLineEdit->show();
    } else {
      this->filterList(3);
      if (hasUser && User::sharedUser()->getAnimeList().count() == 0) {
        this->addNoAnimePrompt();
      }
    }
  });

  // NOLINTNEXTLINE
  QString genrelist = "Action, Adult, Adventure, Cars, Comedy, Dementia, Demons, Doujinshi, Drama, Ecchi, Fantasy, Game, Gender Bender, Harem, Hentai, Historical, Horror, Josei, Kids, Magic, Martial Arts, Mature, Mecha, Military, Motion Comic, Music, Mystery, Mythological , Parody, Police, Psychological, Romance, Samurai, School, Sci-Fi, Seinen, Shoujo, Shoujo Ai, Shounen, Shounen Ai, Slice of Life, Space, Sports, Super Power, Supernatural, Thriller, Tragedy, Vampire, Yaoi, Yuri";
  QStringList genres = genrelist.split(", ");

  for (QString genre : genres) {
    QCheckBox *chk = new QCheckBox();

    chk->setText(genre);
    chk->setTristate(true);

    ui->genreList->addWidget(chk);
  }

  connect(ui->listFilterLineEdit, SIGNAL(textChanged(QString)),
          SLOT(filterList(QString)));
  connect(ui->listFilterLineEdit, SIGNAL(returnPressed()), SLOT(showSearch()));
  connect(ui->listTabs, SIGNAL(currentChanged(int)), SLOT(filterList(int)));

  connect(ui->browseButton, SIGNAL(clicked()), SLOT(loadBrowserData()));

  this->show();
  createActions();
  initTray();
  trayIcon->show();

  int result = API::sharedAPI()->verifyAPI();

  if (result == AniListAPI::OK) {
    connect(API::sharedAPI()->sharedAniListAPI(),
            &AniListAPI::access_granted, [&]() {  // NOLINT
      progress_bar->setValue(10);
      progress_bar->setFormat("Access granted");
      loadUser();

      event_timer->start(1000);
    });

    connect(API::sharedAPI()->sharedAniListAPI(),
            &AniListAPI::access_denied, [&](QString error) {  // NOLINT
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
  delete uptime_timer;

  over->removeAll();

  for (QTableWidgetItem *i : tableWidgets) {
    delete i;
  }

  for (QTableWidgetItem *i : torrentWidgets) {
    delete i;
  }

  User::deleteInstance();
  API::deleteInstance();
  FvUpdater::drop();
}

void MainWindow::closeEvent(QCloseEvent *event) {
  if (trayIcon->isVisible() &&
      settings->getValue(Settings::CloseToTray, true).toBool()) {
    hide();
    event->ignore();
  } else {
    event->ignore();

    this->elegantClose();
  }
}

void MainWindow::mouseDoubleClickEvent(QMouseEvent *event) {
  if (over->containsDrawing("blank_table")) {
    QTableWidget *w = static_cast<QTableWidget *>(
          ui->listTabs->currentWidget()->layout()->itemAt(0)->widget());
    QPoint location = w->mapTo(this, QPoint(0, 0));
    QRect r(location.x()+1, location.y()+24, w->width()-2, w->height()-25);

    if (r.contains(event->x(), event->y())) {
      SearchPanel *sp = new SearchPanel(this);
      sp->show();
    }
  }
}

void MainWindow::changeEvent(QEvent *event) {
  if (event->type() == QEvent::WindowStateChange) {
    if (isMinimized() &&
        settings->getValue(Settings::MinimizeToTray, true).toBool()) {
      this->hide();
      event->ignore();
    }

    if (isMaximized()) {
      showFunc = "showMaximized";
    } else {
      showFunc = "showNormal";
    }
  }
}

void MainWindow::resizeEvent(QResizeEvent *event) {
  over->resize(event->size());

  if (over->containsDrawing("blank_table"))
    this->addSearchPrompt();

  if (over->containsDrawing("no anime"))
    this->addNoAnimePrompt();

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

  int offset = 2;

  #ifdef Q_OS_WIN
    offset = 24;
  #endif

  #ifdef Q_OS_LINUX
    offset = 24;
  #endif

  int notification_count = 0;

  if (this->hasUser) {
    p.drawPixmap(width() - 52, offset, 42, 42, User::sharedUser()->userImage());
    notification_count = User::sharedUser()->notificationCount();
  }

  p.drawRect(width() - 52, offset, 42, 42);

  if (notification_count > 0) {
    p.setBrush(QColor(255, 120, 120));
  } else {
    p.setBrush(QColor(255, 255, 255));
  }

  p.drawEllipse(width() - 20.0f, 33.0f + offset, 17.0f, 17.0f);
  QString notif = notification_count >= 9 ? "9+" :
                                            QString::number(notification_count);
  p.drawText(width() - 19.0f, 33.0f + offset,
             17.0f, 17.0f, Qt::AlignCenter, notif);

  QFont font = p.font();
  font.setPointSize(12);
  p.setFont(font);

  if (this->hasUser) {
    p.drawText(0, offset + 1, width() - 57, 40, Qt::AlignRight,
               User::sharedUser()->displayName());
  }

  event->accept();
}

void MainWindow::showAnimeTab() { ui->tabWidget->setCurrentIndex(0); }
void MainWindow::showTorrentsTab() { ui->tabWidget->setCurrentIndex(1); }
void MainWindow::showAiringTab() {
  ui->tabWidget->setCurrentIndex(2);
  int width = layout->geometry().width();
  int cwidth = layout->contentsWidth();

  if (cwidth < 0) {
    width = this->width() - 2;
    cwidth = this->width() - (this->width() % 200);
  }
  layout->setContentsMargins((width-cwidth)/2, 0, 0, 0);
}
void MainWindow::showStatisticsTab() { ui->tabWidget->setCurrentIndex(3); }
void MainWindow::showBrowseTab() {
  ui->tabWidget->setCurrentIndex(4);
  int width = layout2->geometry().width();
  int cwidth = layout2->contentsWidth();

  if (cwidth < 0) {
    width = this->width() - 2;
    cwidth = this->width() - (this->width() % 200);
  }

  layout2->setContentsMargins((width-cwidth)/2, 0, 0, 0);
}


void MainWindow::showSettings() {
  SettingsDialog *s = new SettingsDialog(this);
  s->show();

  connect(s, &QDialog::accepted, [&] () {
    toggleAnimeRecognition(
          settings->getValue(Settings::AnimeRecognitionEnabled, true).toBool());

    torrents_enabled =
        settings->getValue(Settings::TorrentsEnabled, true).toBool();
  });
}

void MainWindow::showAnimePanel(int row, int column, QTableWidget *source) {
  Q_UNUSED(column);

  QString title = source->item(row, 0)->text();
  QString episodes = source->item(row, 1)->text();
  QString score = source->item(row, 2)->text();
  QString type = source->item(row, source->columnCount() - 1)->text();
  Anime *anime = User::sharedUser()->getAnimeByData(
        title, episodes, score, type);
  QString old_status = anime->getMyStatus();

  if (anime == nullptr) {
    QMessageBox::critical(this, "Shinjiru", tr("Unknown anime."));
    return;
  }


  AnimePanel *ap = new AnimePanel(this, anime, User::sharedUser()->scoreType());

  if (anime->needsLoad() || anime->needsCover()) {
    User::sharedUser()->loadAnimeData(anime, true);
  }

  connect(ap, &AnimePanel::accepted,
          [&, source, anime, row, old_status]() {  // NOLINT
    if (anime->getMyStatus() != old_status) {
      User::sharedUser()->removeFromList(old_status, anime);
      User::sharedUser()->addToList(anime->getMyStatus(), anime);
    }

    User::sharedUser()->animeChanged();
  });

  ap->show();
}

void MainWindow::showAnimePanel(int row, int column) {
  Q_UNUSED(column);

  QTableWidget *source = static_cast<QTableWidget *>(sender());

  showAnimePanel(row, column, source);
}

void MainWindow::viewDashboard() {
  QDesktopServices::openUrl(QString("http://anilist.co/home"));
}

void MainWindow::viewProfile()   {
  QDesktopServices::openUrl(QString("http://anilist.co/user/") +
                            User::sharedUser()->displayName());
}

void MainWindow::viewAnimeList() {
  QDesktopServices::openUrl(QString("http://anilist.co/animelist/") +
                            User::sharedUser()->displayName());
}

void MainWindow::toggleAnimeRecognition(bool checked) {
  if (checked) {
    window_watcher->enable();
  } else {
    window_watcher->disable();
  }

  ui->actionEAR->setChecked(checked);
}

void MainWindow::watch(QString title) {
  if (this->currently_watching != title) {
    this->currently_watching = title;
    this->watch_timer->stop();

    QMap<QString, QString> results = anitomy->parse(title);

    this->cw_title   = results.value("title");
    this->cw_episode = results.value("episode");
    cw_episode = cw_episode.remove(QRegExp("^[0]*"));

    if (cw_episode.isEmpty() || cw_title.isEmpty()) {
        return;
    }

    cw_anime = User::sharedUser()->getAnimeByTitle(cw_title, true);

    if (cw_anime == 0 || cw_anime->getTitle().isEmpty()) {
      QJsonObject results = API::sharedAPI()->sharedAniListAPI()->get(
            API::sharedAPI()->sharedAniListAPI()->API_ANIME_SEARCH(cw_title))
              .array().at(0).toObject();
      cw_anime = User::sharedUser()->getAnimeByTitle(
            results.value("title_romaji").toString());
      if (cw_anime == 0) return;
    }

    cw_episode = QString::number(cw_episode.toInt() -
                                 cw_anime->getUpdateOffset());

    if (cw_anime->getMyProgress() >= cw_episode.toInt()) {
      return;
    }

    if (cw_anime->getMyStatus() != "watching" &&
        cw_anime->getMyStatus() != "plan to watch") {
      return;
    }

    int auto_update_delay =
        settings->getValue(Settings::AutoUpdateDelay, 120).toInt();

    this->watch_timer->start(1000 * auto_update_delay);

    if(settings->getValue(Settings::AnimeDetectNotify, true).toBool())
      this->trayIcon->showMessage("Shinjiru",
                                  tr("Updating %1 to episode %2 in %3 seconds")
                                  .arg(cw_anime->getTitle())
                                  .arg(cw_episode)
                                  .arg(QString::number(auto_update_delay)));
  }
}

void MainWindow::updateEpisode() {
  QMap<QString, QString> data;
  data.insert("id", cw_anime->getID());
  data.insert("episodes_watched", cw_episode);
  data.insert("list_status", "watching");

  cw_episode += cw_anime->getUpdateOffset();

  cw_anime->setMyProgress(cw_episode.toInt());

  if (cw_anime->getMyProgress() == cw_anime->getEpisodeCount() &&
      cw_anime->getEpisodeCount() != 0) {
    data.insert("list_status", "completed");
    cw_anime->setMyStatus("completed");
  }

  userListLoaded();

  QJsonObject response = API::sharedAPI()->sharedAniListAPI()->put(
        API::sharedAPI()->sharedAniListAPI()->API_EDIT_LIST, data).object();

  if(!settings->getValue(Settings::AnimeUpdateNotify, true).toBool()) return;

  if (response != QJsonObject()) {
    trayIcon->showMessage("Shinjiru", tr("%1 updated to episode %2")
                          .arg(cw_anime->getTitle())
                          .arg(cw_episode));
    if (data.value("list_status") == "completed") {
      trayIcon->showMessage("Shinjiru", tr("%1 marked as completed")
                            .arg(cw_anime->getTitle()));
    }
  } else {
    trayIcon->showMessage("Shinjiru", tr("%1 was not successfully updated")
                          .arg(cw_anime->getTitle()));
  }
}

void MainWindow::eventTick() {
  if(torrents_enabled) {
    if (torrent_refresh_time == 0) {
      refreshTorrentListing();
    }

    torrent_refresh_time--;

    ui->refreshButton->setText(tr("Refresh (%1)")
                               .arg(QString::number(torrent_refresh_time)));
  } else {
    ui->refreshButton->setText(tr("Refresh"));
  }

  if (user_refresh_time == 0) {
    user_refresh_time = default_time;
    loadUser();
  }

  for (int i = 0; i < airing_anime.count(); i++) {
      AiringAnime *aa = airing_anime.at(i);

      if (!aa->getAnime()->needsLoad())
        aa->tick();
  }

  user_refresh_time--;

  qint64 seconds = uptime_timer->elapsed() / 1000;
  int minutes = seconds / 60;
  int hours = seconds / 60.0 / 60;
  int days = seconds / 60.0 / 60.0 / 24;

  seconds %= 60;
  minutes %= 60;
  hours %= 24;

  QString format = "";
  QString comma = "";

  if (days > 1) {
    format += QString::number(days) + tr(" days");
  } else if (days > 0) {
    format += QString::number(days) + tr(" day");
  }

  if (days > 0) comma = ", ";

  if (hours > 1) {
    format += comma + QString::number(hours) + tr(" hours");
  } else if (hours > 0) {
    format += comma + QString::number(hours) + tr(" hour");
  }

  if (hours > 0) comma = ", ";

  if (days == 0) {
    if (minutes > 1) {
      format += comma + QString::number(minutes) + tr(" minutes");
    } else if (minutes > 0) {
      format += comma + QString::number(minutes) + tr(" minute");
    }
  }

  if (minutes > 0) comma = ", ";

  if (hours == 0) {
    if (seconds == 1) {
      format += comma + QString::number(seconds) + tr(" second");
    } else {
      format += comma + QString::number(seconds) + tr(" seconds");
    }
  }

  ui->lblUptime->setText(format);
}

void MainWindow::exportListJSON() {
  if (!this->hasUser) return;

  QString fileName =
      QFileDialog::getSaveFileName(this, tr("Save File"),
                                   qApp->applicationDirPath(),
                                   tr("Json File (*.json)"));

  if (fileName.isEmpty()) return;
  QFile f(fileName);
  f.open(QFile::WriteOnly);
  if (f.isOpen()) {
    QByteArray data = User::sharedUser()->listJson();
    int bytes = f.write(data);

    if (bytes != data.length() || bytes == -1) {
      QMessageBox::information(this, "Shinjiru",
                               tr("Error %d: File could not be written to")
                               .arg(bytes));
    }

    f.close();
  }
}


void MainWindow::showSearch() {
  SearchPanel *sp = new SearchPanel(this);
  sp->setSearch(ui->listFilterLineEdit->text());
  sp->show();
}

void MainWindow::elegantClose() {
  QSettings s;
  s.setValue("mainWindowGeometry", saveGeometry());
  s.setValue("mainWindowState", saveState());

  for (int i = 0; i < ui->listTabs->count(); i++) {
    QTableWidget *t = static_cast<QTableWidget *>(
          ui->listTabs->widget(i)->layout()->itemAt(0)->widget());

    QString key = ui->listTabs->tabText(i).replace(QRegExp("[ ]+"), "")
                                          .replace(QRegExp("\\([0-9]+\\)"), "")
                                          + "State";

    s.setValue(key, t->horizontalHeader()->saveState());
  }

  for (QFuture<void> f : this->async_registry) {
    if (f.isRunning()) {
      qApp->processEvents();
      f.waitForFinished();
    }
  }

  if (this->hasUser) {
    connect(User::sharedUser(), SIGNAL(quitFinished()), qApp, SLOT(quit()));
    User::sharedUser()->quit();
  } else {
    qApp->quit();
  }
}

void MainWindow::reloadSmartTitles() {
  QList<SmartTitle *> smart_titles;

  User::sharedUser()->clearSmartTitles();

  QFile smart_file(QCoreApplication::applicationDirPath() + "/relations.json");
  if (!smart_file.open(QFile::ReadOnly)) return;

  QJsonArray relations = QJsonDocument::fromJson(smart_file.readAll()).array();

  for (QJsonValue v : relations) {
    QJsonObject relation = v.toObject();

    SmartTitle *s = new SmartTitle(0);
    s->setID(relation.value("id").toString("0"));
    s->setCustom(relation.value("custom").toString());
    s->setTitle(relation.value("title").toString());

    if (relation.contains("offset")) {
      s->setOffset(relation.value("offset").toString().toInt());
    }

    smart_titles.append(s);
  }

  User::sharedUser()->setSmartTitles(smart_titles);

  qDebug() << "Loaded" << smart_titles.length() << "smart titles";
}
