#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDesktopServices>

#include "../app.h"
#include "../api/api.h"
#include "animepanel.h"

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  QFile styleFile(":/style.css");
  styleFile.open(QFile::ReadOnly);
  QString styleSheet = QLatin1String(styleFile.readAll());
  qApp->setStyleSheet(styleSheet);

  user                = nullptr;
  awesome             = new QtAwesome(qApp);
  api                 = new AniListAPI(this, api_id, api_secret);
  settings            = new Settings(this);
  window_watcher      = new WindowWatcher(this);
  anitomy             = new AnitomyWrapper();

  awesome->initFontAwesome();

  event_timer         = new QTimer(this);
  watch_timer         = new QTimer(this);
  progress_bar        = new QProgressBar(ui->statusBar);

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

  connect(ui->actionExit,     &QAction::triggered,     []() {exit(0);});
  connect(ui->actionAbout,    &QAction::triggered,     []() {return;});
  connect(ui->actionHelp,     &QAction::triggered,     []() {return;});
  connect(ui->actionRL,       &QAction::triggered,     [&]() {loadUserList();});
  connect(ui->actionVAL,      SIGNAL(triggered()),     SLOT(viewAnimeList()));
  connect(ui->actionVD,       SIGNAL(triggered()),     SLOT(viewDashboard()));
  connect(ui->actionVP,       SIGNAL(triggered()),     SLOT(viewProfile()));
  connect(ui->actionEAR,      SIGNAL(triggered(bool)), SLOT(toggleAnimeRecognition(bool)));

  connect(window_watcher, SIGNAL(title_found(QString)), SLOT(watch(QString)));
  connect(watch_timer,    SIGNAL(timeout()),            SLOT(updateEpisode()));

  API *instance = new API(this);
  int result = instance->verify(api);
  delete instance;
  instance = nullptr;

  if(result == AniListAPI::OK) {
    connect(api, &AniListAPI::access_granted, [&]() {
      progress_bar->setValue(10);
      progress_bar->setFormat("Access granted");
      loadUser();
    });
  }
}

MainWindow::~MainWindow() {
  delete ui;
  delete awesome;
  delete api;
  delete anitomy;
}

void MainWindow::closeEvent(QCloseEvent *event) {
  settings->write_settings();
  event->accept();
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
  Anime *anime = user->getAnimeByTitle(title);

  AnimePanel *ap = new AnimePanel(this, anime, user->scoreType(), api);

  if(anime->needsLoad()) {
    user->loadAnimeData(anime);
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
}

void MainWindow::watch(QString title) {
  if(this->currently_watching != title) {
    this->currently_watching = title;
    this->watch_timer->stop();

    QMap<QString, QString> results = anitomy->parse(title);

    this->cw_title   = results.value("title");
    this->cw_episode = results.value("episode");

    if(cw_title.isEmpty() || cw_episode.isEmpty()) {
        return;
    }

    this->watch_timer->start(1000 * 60 * 2); // Start timer to expire after 2 minutes
  }
}

void MainWindow::updateEpisode() {
  Anime *anime = this->user->getAnimeByTitle(cw_title);

  if(anime->getMyProgress() > cw_episode.toInt()) {
    return;
  }

  QMap<QString, QString> data;
  data.insert("id",               anime->getID());
  data.insert("episodes_watched", cw_episode);

  api->put(api->API_EDIT_LIST, data);
}
