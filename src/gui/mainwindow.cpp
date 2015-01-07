#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDesktopServices>
#include <regex>

#include "../app.h"
#include "../api/api.h"
#include "animepanel.h"
#include "../lib/torrentrss.h"

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  QFile styleFile(":/style.css");
  styleFile.open(QFile::ReadOnly);
  QString styleSheet = QLatin1String(styleFile.readAll());
  qApp->setStyleSheet(styleSheet);

  user                 = nullptr;
  awesome              = new QtAwesome(qApp);
  api                  = new AniListAPI(this, api_id, api_secret);
  settings             = new Settings(this);
  window_watcher       = new WindowWatcher(this);
  anitomy              = new AnitomyWrapper();
  event_timer          = new QTimer(this);
  watch_timer          = new QTimer(this);
  progress_bar         = new QProgressBar(ui->statusBar);
  torrent_refresh_time = 0;

  awesome->initFontAwesome();


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
  connect(event_timer,    SIGNAL(timeout()),            SLOT(eventTick()));

  connect(ui->torrentTable,    SIGNAL(customContextMenuRequested(QPoint)), SLOT(torrentContextMenu(QPoint)));
  connect(ui->torrentFilter,   SIGNAL(textChanged(QString)),               SLOT(filterTorrents(QString)));
  connect(ui->chkHideUnknown,  SIGNAL(toggled(bool)),                      SLOT(filterTorrents(bool)));

  this->show();

  API *instance = new API(this);
  int result = instance->verify(api);
  delete instance;
  instance = nullptr;

  if(result == AniListAPI::OK) {
    connect(api, &AniListAPI::access_granted, [&]() {
      progress_bar->setValue(10);
      progress_bar->setFormat("Access granted");
      loadUser();
      event_timer->start(1000);
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

void MainWindow::eventTick() {
  if(torrent_refresh_time == 0) {
    torrent_refresh_time = 60;
    refreshTorrentListing();
  }

  torrent_refresh_time--;
  ui->refreshButton->setText("Refresh (" + QString::number(torrent_refresh_time) + ")");

  event_timer->start(1000);
}

void MainWindow::refreshTorrentListing() {
  TorrentRSS *torrents = new TorrentRSS(this);
  QEventLoop rssLoop;
  connect(torrents, SIGNAL(done()), &rssLoop, SLOT(quit()));
  torrents->fetch();
  rssLoop.exec();

  QStringList titles = QStringList(*(torrents->getTitles()));
  QStringList links = QStringList(*(torrents->getLinks()));
  int offset = 0;

  for(int i = 0; i < titles.length(); i++) {
    if(ui->torrentTable->rowCount() <= i)
      ui->torrentTable->insertRow(i);

    QMap<QString, QString> result;

    try {
      result = anitomy->parse(titles.at(i));
    } catch(std::regex_error& e) {
      qDebug() << "Error parsing: " << titles.at(i);
      offset++;
      continue;
    }


    QString parsedTitle   = result.value("title");
    QString episodeNumber = result.value("episode");
    QString subGroup      = result.value("subs");
    QString videoType     = result.value("res");

    QTableWidgetItem *titleItem = new QTableWidgetItem(parsedTitle);
    QTableWidgetItem *epItem = new QTableWidgetItem(episodeNumber);
    QTableWidgetItem *subItem = new QTableWidgetItem(subGroup);
    QTableWidgetItem *videoItem = new QTableWidgetItem(videoType);
    QTableWidgetItem *fileNameItem = new QTableWidgetItem(titles.at(i));
    QTableWidgetItem *linkItem = new QTableWidgetItem(links.at(i));

    if(episodeNumber == "") {
      qDebug() << "Unknown episode for: " << parsedTitle << ", skipping";
      offset++;
      continue;
    }

    if(links.at(i) == "") {
      qDebug() << "Unknown link for: " << parsedTitle << ", skipping";
      offset++;
      continue;
    }

    ui->torrentTable->setItem(i - offset, 0, titleItem);
    ui->torrentTable->setItem(i - offset, 1, epItem);
    ui->torrentTable->setItem(i - offset, 2, subItem);
    ui->torrentTable->setItem(i - offset, 3, videoItem);
    ui->torrentTable->setItem(i - offset, 4, fileNameItem);
    ui->torrentTable->setItem(i - offset, 5, linkItem);
  }

  while(offset > 0) {
    ui->torrentTable->removeRow(ui->torrentTable->rowCount() - 1);
    offset--;
  }

  filterTorrents(ui->torrentFilter->text(), ui->chkHideUnknown->isChecked());
}

void MainWindow::torrentContextMenu(QPoint pos) {
  QTableWidgetItem *item = ui->torrentTable->itemAt(pos);
  int row = item->row();
  pos.setY(pos.y() + 120);
  QAction *pDownloadAction = new QAction("Download",ui->torrentTable);
  QAction *pRuleAction = new QAction("Create rule",ui->torrentTable);

  QSignalMapper *signalMapper1 = new QSignalMapper(this);
  QSignalMapper *signalMapper2 = new QSignalMapper(this);

  signalMapper1->setMapping(pDownloadAction, row);
  signalMapper2->setMapping(pRuleAction, row);

  connect(pDownloadAction, SIGNAL(triggered()), signalMapper1, SLOT (map()));
  connect(signalMapper1, SIGNAL(mapped(int)), this, SLOT(download(int)));

  connect(pRuleAction, SIGNAL(triggered()), signalMapper2, SLOT (map()));
  connect(signalMapper2, SIGNAL(mapped(int)), this, SLOT(createRule(int)));

  QMenu *pContextMenu = new QMenu( this);
  pContextMenu->addAction(pDownloadAction);
  pContextMenu->addAction(pRuleAction);
  pContextMenu->exec(mapToGlobal(pos));
  delete pContextMenu;
  delete signalMapper1;
  delete signalMapper2;
  signalMapper1 = NULL;
  signalMapper2 = NULL;
  pContextMenu = NULL;
}

void MainWindow::download(int row) {
    QDesktopServices::openUrl(ui->torrentTable->item(row, 4)->text());
}

void MainWindow::createRule(int row) {
    qDebug() << "Creating rule " << row;
}


void MainWindow::filterTorrents(QString text, bool checked) {
  for(int i = 0; i < ui->torrentTable->rowCount(); i++)
    ui->torrentTable->hideRow(i);

  QList<QTableWidgetItem *> items = ui->torrentTable->findItems(text, Qt::MatchContains);

  for(int i = 0; i < items.count(); i++) {
    if(items.at(i)->column() != 0 ) continue;
    bool show = true;

    if(checked) {
      QString f_title = items.at(i)->text();
      if(user->getAnimeByTitle(f_title)->getAiringStatus() != "currently airing") show = false;
    }

    if(show)
      ui->torrentTable->showRow(items.at(i)->row());
  }
}

void MainWindow::filterTorrents(bool checked) {
  filterTorrents(ui->torrentFilter->text(), checked);
}

void MainWindow::filterTorrents(QString text) {
  filterTorrents(text, ui->chkHideUnknown->isChecked());
}
