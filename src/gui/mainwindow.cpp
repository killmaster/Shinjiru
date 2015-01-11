#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDesktopServices>
#include <QIcon>

#include "../app.h"
#include "../api/api.h"
#include "animepanel.h"
#include "fvupdater.h"
#include "about.h"

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

  user                 = nullptr;
  awesome              = new QtAwesome(qApp);
  settings             = new Settings(this);
  window_watcher       = new WindowWatcher(this);
  anitomy              = new AnitomyWrapper();
  event_timer          = new QTimer(this);
  watch_timer          = new QTimer(this);
  progress_bar         = new QProgressBar(ui->statusBar);
  torrent_refresh_time = 0;

  awesome->initFontAwesome();
  loadSettings();

  QFont font = ui->listTabs->tabBar()->font();
  font.setCapitalization(QFont::Capitalize);
  ui->listTabs->tabBar()->setFont(font);

  QWidget *container = new QWidget(ui->scrollArea);
  layout = new FlowLayout(container);
  ui->scrollArea->setWidget(container);
  container->setLayout(layout);

  QVariantMap black;
  black.insert("color", QColor(0, 0, 0));
  black.insert("color-active", QColor(0, 0, 0));
  black.insert("color-disabled", QColor(0, 0, 0));
  black.insert("color-selected", QColor(0, 0, 0));

  ui->airingButton    ->setIcon(awesome->icon(fa::clocko,   black));
  ui->torrentsButton  ->setIcon(awesome->icon(fa::rss,      black));
  ui->animeButton     ->setIcon(awesome->icon(fa::bars,     black));
  ui->statisticsButton->setIcon(awesome->icon(fa::piechart, black));

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
  connect(ui->actionHelp,   &QAction::triggered,                             [&]() {showAbout();});
  connect(ui->actionRL,     &QAction::triggered,                             [&]() {loadUserList();});
  connect(ui->actionVAL,    SIGNAL(triggered()),                             SLOT(viewAnimeList()));
  connect(ui->actionVD,     SIGNAL(triggered()),                             SLOT(viewDashboard()));
  connect(ui->actionVP,     SIGNAL(triggered()),                             SLOT(viewProfile()));
  connect(ui->actionEAR,    SIGNAL(triggered(bool)),                         SLOT(toggleAnimeRecognition(bool)));
  connect(ui->actionUpdate, SIGNAL(triggered()), FvUpdater::sharedUpdater(), SLOT(CheckForUpdatesNotSilent()));

  connect(ui->disconnectButton, SIGNAL(clicked()), SLOT(resetAPI()));

  connect(window_watcher, SIGNAL(title_found(QString)), SLOT(watch(QString)));
  connect(watch_timer,    SIGNAL(timeout()),            SLOT(updateEpisode()));
  connect(event_timer,    SIGNAL(timeout()),            SLOT(eventTick()));

  connect(ui->torrentTable,    SIGNAL(customContextMenuRequested(QPoint)), SLOT(torrentContextMenu(QPoint)));
  connect(ui->torrentFilter,   SIGNAL(textChanged(QString)),               SLOT(filterTorrents(QString)));
  connect(ui->chkHideUnknown,  SIGNAL(toggled(bool)),                      SLOT(filterTorrents(bool)));
  connect(ui->refreshButton,   SIGNAL(clicked()),                          SLOT(refreshTorrentListing()));

  this->show();
  createActions();
  initTray();
  trayIcon->show();

  API *instance = API::sharedAPI();
  int result = instance->verify();

  if(result == AniListAPI::OK) {
    connect(API::sharedAPI()->sharedAniListAPI(), &AniListAPI::access_granted, [&]() {
      progress_bar->setValue(10);
      progress_bar->setFormat("Access granted");
      loadUser();
      reloadRules();
      event_timer->start(1000);
    });

    connect(API::sharedAPI()->sharedAniListAPI(), &AniListAPI::access_denied, [&](QString error) {
      QMessageBox::critical(this, "Shinjiru", "Error: " + error);
    });
  }
}

MainWindow::~MainWindow() {
  delete ui;
  delete awesome;
  delete anitomy;
}

void MainWindow::closeEvent(QCloseEvent *event) {
  if(trayIcon->isVisible()) {
    hide();
    event->ignore();
  }
}

void MainWindow::changeEvent(QEvent *event) {
    if(event->type() == QEvent::WindowStateChange) {
        if(isMinimized())
            this->hide();
            event->ignore();
    }
}


void MainWindow::resizeEvent(QResizeEvent *event) {
  event->accept();

  layout->setContentsMargins(layout->horizontalSpacing()/2, 0, 0, 0);
}

void MainWindow::paintEvent(QPaintEvent *event) {
  QPainter p(this);

  QFont font = p.font();
  font.setPointSize(14);
  p.setFont(font);

  if(user != nullptr) {
    p.drawText(0, 30, width() - 60, 40, Qt::AlignRight, user->displayName());
    p.drawPixmap(width() - 55, 25, 48, 48, user->userImage());
  }

  p.drawRect(width() - 55, 25, 48, 48);

  event->accept();
}

void MainWindow::showAnimeTab()      { ui->tabWidget->setCurrentIndex(0); }
void MainWindow::showSettingsTab()   { ui->tabWidget->setCurrentIndex(1); }
void MainWindow::showTorrentsTab()   { ui->tabWidget->setCurrentIndex(2); }
void MainWindow::showAiringTab()     { ui->tabWidget->setCurrentIndex(3); }
void MainWindow::showStatisticsTab() { ui->tabWidget->setCurrentIndex(4); }

void MainWindow::showAnimePanel(int row, int column) {
  Q_UNUSED(column);

  QTableWidget *source = static_cast<QTableWidget *>(sender());
  QString title = source->item(row, 0)->text();
  QString episodes = source->item(row, 1)->text();
  QString score = source->item(row, 2)->text();
  QString type = source->item(row, 3)->text();
  Anime *anime = user->getAnimeByData(title, episodes, score, type);

  AnimePanel *ap = new AnimePanel(this, anime, user->scoreType());

  if(anime->needsLoad() || anime->needsCover()) {
    user->loadAnimeData(anime, true);
  }

  ap->show();
}

void MainWindow::viewDashboard() { QDesktopServices::openUrl(QString("http://anilist.co/home")); }
void MainWindow::viewProfile()   { QDesktopServices::openUrl(QString("http://anilist.co/user/") + user->displayName()); }
void MainWindow::viewAnimeList() { QDesktopServices::openUrl(QString("http://anilist.co/animelist/") + user->displayName()); }

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

    if(cw_episode.isEmpty() || cw_title.isEmpty()) {
        return;
    }

    cw_anime = this->user->getAnimeByTitle(cw_title);

    if(cw_anime->getTitle().isEmpty()) {
      QJsonObject results = API::sharedAPI()->sharedAniListAPI()->get(API::sharedAPI()->sharedAniListAPI()->API_ANIME_SEARCH(cw_title)).array().at(0).toObject();
      user->getAnimeByTitle(results.value("title_romaji").toString());
    }

    if(cw_anime->getMyProgress() > cw_episode.toInt()) {
      return;
    }

    if(cw_anime->getMyStatus() != "watching" || cw_anime->getMyStatus() != "plan to watch") {
      return;
    }

    this->watch_timer->start(1000 * 60 * 2); // Start timer to expire after 2 minutes
    this->trayIcon->showMessage("Shinjiru", "Updating " + cw_anime->getTitle() + " to episode " + cw_episode + " in 2 minutes");
  }
}

void MainWindow::updateEpisode() {
  QMap<QString, QString> data;
  data.insert("id",               cw_anime->getID());
  data.insert("episodes_watched", cw_episode);
  data.insert("list_status",      "watching");

  cw_anime->setMyProgress(cw_episode.toInt());
  userListLoaded();

  trayIcon->showMessage("Shinjiru", cw_anime->getTitle() + " updated to episode " + cw_episode);

  API::sharedAPI()->sharedAniListAPI()->put(API::sharedAPI()->sharedAniListAPI()->API_EDIT_LIST, data);
}

void MainWindow::eventTick() {
  if(torrent_refresh_time == 0) {
    torrent_refresh_time = torrent_interval;
    refreshTorrentListing();
  }

  torrent_refresh_time--;
  ui->refreshButton->setText("Refresh (" + QString::number(torrent_refresh_time) + ")");
  event_timer->start(1000);
}

void MainWindow::showAbout() {
  About *about = new About(this);
  about->show();
}

void MainWindow::resetAPI() {
  API::sharedAPI()->sharedAniListAPI()->setAuthorizationCode("");

  settings->setValue(Settings::AniListAccess, "");
  settings->setValue(Settings::AniListExpires, QDateTime::currentDateTimeUtc());
  settings->setValue(Settings::AniListRefresh, "");

  QProcess::startDetached(QApplication::applicationFilePath());
  exit(0);
}
