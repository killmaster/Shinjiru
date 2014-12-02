#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSettings>
#include <QFile>
#include <QVariantMap>
#include <QDebug>
#include <QImage>
#include <QPixmap>
#include <QTableWidgetItem>
#include <QTimer>
#include <QSignalMapper>
#include <QDesktopServices>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>
#include <QtConcurrent/QtConcurrentMap>
#include <QInputDialog>
#include <QTextDocument>
#include <QProgressBar>

#include <string>
#include <regex>

#include "QtAwesome.h"
#include "app.h"
#include "torrents.h"
#include "anilistapi.h"
#include "anime.h"

#include "anitomy/anitomy/anitomy.h"


MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow) {
  /*
   * Setup window and application preferences
   */
  ui->setupUi(this);

  QFile styleFile(":/style.css"); styleFile.open(QFile::ReadOnly);
  QString styleSheet = QLatin1String(styleFile.readAll()); qApp->setStyleSheet(styleSheet);

  awesome             = new QtAwesome(qApp);             awesome->initFontAwesome();
  torrentRefreshTimer = new QTimer(this);
  eventTimer          = new QTimer(this);
  progressBar         = new QProgressBar(ui->statusBar); ui->statusBar->addWidget(progressBar);
  api                 = new AniListAPI(this, api_id, api_secret);

  QFont font = ui->listTabs->tabBar()->font();
  font.setCapitalization(QFont::Capitalize);
  ui->listTabs->tabBar()->setFont(font);

  QVariantMap black;
  black.insert("color", QColor(0, 0, 0));
  black.insert("color-active", QColor(0, 0, 0));
  black.insert("color-disabled", QColor(0, 0, 0));
  black.insert("color-selected", QColor(0, 0, 0));

  ui->currentlyAiringButton->setIcon(awesome->icon(fa::clocko,   black));
  ui->torrentsButton       ->setIcon(awesome->icon(fa::rss,      black));
  ui->animeButton          ->setIcon(awesome->icon(fa::bars,     black));
  ui->statisticsButton     ->setIcon(awesome->icon(fa::piechart, black));

  ui->tabWidget->tabBar()->hide();
  ui->tabWidget->setCurrentIndex(0);
  ui->listTabs->setCurrentIndex(0);
  progressBar->setRange(0, 100);

  readSettings();

  /*
   * Connect signals to slots
   */
  connect(this,                SIGNAL(displayNameAvailable()),             SLOT(loadList()));

  connect(ui->actionSettings,  SIGNAL(triggered()),                        SLOT(showSettingsTab()));
  connect(ui->applyButton,     SIGNAL(clicked()),                          SLOT(applySettings()));

  connect(ui->animeButton,     SIGNAL(clicked()),                          SLOT(showAnimeTab()));

  connect(ui->torrentsButton,  SIGNAL(clicked()),                          SLOT(showTorrentsTab()));
  connect(ui->torrentFilter,   SIGNAL(textChanged(QString)),               SLOT(filterTorrents(QString)));
  connect(ui->chkHideUnknown,  SIGNAL(toggled(bool)),                      SLOT(filterTorrents(bool)));
  connect(ui->refreshButton,   SIGNAL(clicked()),                          SLOT(loadTorrents()));
  connect(torrentRefreshTimer, SIGNAL(timeout()),                          SLOT(loadTorrents()));
  connect(eventTimer,          SIGNAL(timeout()),                          SLOT(tick()));
  connect(ui->torrentTable,    SIGNAL(customContextMenuRequested(QPoint)), SLOT(torrentContextMenu(QPoint)));

  connect(&userWatcher,        SIGNAL(finished()),                         SLOT(refreshUser()));
  connect(&userListWatcher,    SIGNAL(finished()),                         SLOT(refreshList()));
  connect(&animeFutureData,    SIGNAL(finished()),                         SLOT(resetProgress()));

  connect(ui->actionVAL,       SIGNAL(triggered()),                        SLOT(viewAnimeList()));
  connect(ui->actionVP,        SIGNAL(triggered()),                        SLOT(viewProfile()));
  connect(ui->actionVD,        SIGNAL(triggered()),                        SLOT(viewDashboard()));

  /*
   * Connect to AniList and fetch the user
   */
  progressBar->setValue(5);
  progressBar->setFormat("Authorizing");

  if(!api->hasAuthorizationCode()) {
    bool ok;
    QDesktopServices::openUrl(QUrl("http://auth.shinjiru.me"));
    QString message = "Authorization code:                                                                                    ";
    QString text = QInputDialog::getText(this, tr("Authorization Code Request"), tr(message.toUtf8().data()), QLineEdit::Normal, "", &ok);

    if (ok && !text.isEmpty()) {
      api->setAuthorizationCode(text);
    } else {
      exit(NO_AUTHCODE);
    }
  }

  if (api->init() == AniListAPI::OK) {
    QEventLoop waitForInit;
    connect(api, SIGNAL(access_granted()), &waitForInit, SLOT(quit()));
    connect(api, SIGNAL(access_denied()), &waitForInit, SLOT(quit()));
    waitForInit.exec();

    progressBar->setValue(20);
    progressBar->setFormat("Loading user");

    loadUser();
    ui->currentlyWatchingTable->resizeColumnsToContents();
  } else {
    exit(API_ERROR);
  }

  loadTorrents();
  ui->torrentTable->resizeColumnsToContents();
  eventTimer->start(1000);
}

MainWindow::~MainWindow() {
  delete ui;
  delete awesome;
  delete api;
  delete torrentRefreshTimer;
  delete eventTimer;
}

void MainWindow::closeEvent(QCloseEvent *event) {
  writeSettings();
  event->accept();
}

void MainWindow::paintEvent(QPaintEvent *event) {
  QPainter p(this);

  // Draw the username on the screen
  QFont font = p.font();
  font.setPointSize(14);
  p.setFont(font);
  p.drawText(0, 30, width() - 60, 40, Qt::AlignRight, aniListDisplayName);

  // Draw the userimage on the screen
  p.drawPixmap(width() - 55, 25, 48, 48, userImage);
  p.drawRect  (width() - 55, 25, 48, 48);

  event->accept();
}

void MainWindow::readSettings() {
  QSettings settings;

  QPoint pos                 = settings.value("window/pos", QPoint(200, 200)).toPoint();
  QSize size                 = settings.value("window/size", QSize(800, 600)).toSize();
  bool wasMaximized          = settings.value("window/maximized", false).toBool();
  int torrentRefreshInterval = settings.value("tinterval", 3600).toInt();

  torrentRefreshTimer->setInterval(torrentRefreshInterval);

  resize(size);
  move(pos);

  if(wasMaximized)
    showMaximized();
}

void MainWindow::writeSettings() {
  QSettings settings;

  if(isMaximized()) {
    settings.setValue("window/maximized", true);
  } else {
    settings.setValue("window/pos", pos());
    settings.setValue("window/size", size());
    settings.setValue("window/maximized", false);
  }

  settings.setValue("tinterval", 3600 * 1000);
}

void MainWindow::showAnimeTab()    { ui->tabWidget->setCurrentIndex(0); }
void MainWindow::showSettingsTab() { ui->tabWidget->setCurrentIndex(1); }
void MainWindow::showTorrentsTab() { ui->tabWidget->setCurrentIndex(2); }

void MainWindow::enableApply()   { ui->applyButton->setEnabled(true); }
void MainWindow::applySettings() { writeSettings(); }

void MainWindow::viewDashboard() { QDesktopServices::openUrl(QString("http://anilist.co/home")); }
void MainWindow::viewProfile()   { QDesktopServices::openUrl(QString("http://anilist.co/user/") + aniListDisplayName); }
void MainWindow::viewAnimeList() { QDesktopServices::openUrl(QString("http://anilist.co/animelist/") + aniListDisplayName); }

void MainWindow::resetProgress() { progressBar->setFormat(""); progressBar->setValue(0); }
void MainWindow::updateProgess() { progressBar->setValue(progressBar->value() + 1); }

void MainWindow::loadTorrents() {
  torrentRefreshTimer->stop();
  ui->refreshButton->setText("Refresh (0)");
  Torrents *torrents = new Torrents(this);
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

    anitomy::Anitomy anitomy;
    std::basic_string<wchar_t> title = toAnitomyFormat(titles.at(i));

    try {
      anitomy.Parse(title);
    } catch(std::regex_error& e) {
      qDebug() << "Error parsing: " << QString::fromWCharArray(title.c_str());
      offset++;
      continue;
    }

    auto& elements = anitomy.elements();

    QString parsedTitle = QString::fromWCharArray(
                elements.get(anitomy::kElementAnimeTitle).c_str());
    QString episodeNumber = QString::fromWCharArray(
                elements.get(anitomy::kElementEpisodeNumber).c_str());
    QString subGroup = QString::fromWCharArray(
                elements.get(anitomy::kElementReleaseGroup).c_str());
    QString videoType = QString::fromWCharArray(
                elements.get(anitomy::kElementVideoResolution).c_str());

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

  torrentRefreshTimer->start();
  filterTorrents(ui->torrentFilter->text(), ui->chkHideUnknown->isChecked());
  filterTorrents(ui->chkHideUnknown->isChecked());
}

void MainWindow::tick() {
  int remainingTime = torrentRefreshTimer->remainingTime() / 1000;
  ui->refreshButton->setText("Refresh (" + QString::number(remainingTime) + ")");
  eventTimer->start(1000);
}

void MainWindow::filterTorrents(QString text, bool checked) {
  for(int i = 0; i < ui->torrentTable->rowCount(); i++)
    ui->torrentTable->hideRow(i);

  QList<QTableWidgetItem *> items =
    ui->torrentTable->findItems(text, Qt::MatchContains);

  for(int i = 0; i < items.count(); i++) {
    if(items.at(i)->column() != 0 ) continue;
    int count = 1;

    if(checked) {
      QString f_title = items.at(i)->text();
      Qt::MatchFlag flag = Qt::MatchExactly;
      count = ui->currentlyWatchingTable->findItems(f_title, flag).count() +
          ui->completedTable->findItems(f_title, flag).count() +
          ui->onHoldTable->findItems(f_title, flag).count() +
          ui->planToWatchTable->findItems(f_title, flag).count() +
          ui->droppedTable->findItems(f_title, flag).count();
    }

    if(count > 0)
      ui->torrentTable->showRow(items.at(i)->row());
  }
}

void MainWindow::filterTorrents(bool checked) {
  filterTorrents(ui->torrentFilter->text(), checked);
}

void MainWindow::filterTorrents(QString text) {
  filterTorrents(text, ui->chkHideUnknown->isChecked());
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

std::basic_string<wchar_t> MainWindow::toAnitomyFormat(QString text) {
  char* s = text.toLocal8Bit().data();
  std::wstring w(s, s+strlen(s));
  return w.c_str();
}

void MainWindow::loadUser() {
  userJson = QtConcurrent::run([&]() {
    return api->get(api->API_USER);
  });

  userWatcher.setFuture(userJson);
}

void MainWindow::refreshUser() {
  QJsonObject userData = userJson.result();
  this->aniListDisplayName = userData.value("display_name").toString();
  repaint();
  score_type = userData.value("score_type").toInt();
  switch(score_type) {
    case 0:
      max_score = "10"; break;
    case 1:
      max_score = "100"; break;
    case 2:
      max_score = "5"; break;
    case 3:
      max_score = "3"; break;
    case 4:
      max_score = "10.0"; break;
  };

  progressBar->setValue(30);
  progressBar->setFormat("Loading user image");

  emit displayNameAvailable();
  QUrl imageUrl(userData.value("image_url_med").toString());
  userImageCtrl = new FileDownloader(imageUrl, this);
  connect(userImageCtrl, SIGNAL(downloaded()), SLOT(setUserImage()));
}

void MainWindow::setUserImage() {
  QPixmap u_image;
  u_image.loadFromData(userImageCtrl->downloadedData());
  this->userImage = u_image;
  progressBar->setValue(40);
  repaint();
}

void MainWindow::loadList() {
  progressBar->setValue(40);
  progressBar->setFormat("Loading anime list");

  userListJson = QtConcurrent::run([&]() {
    return api->get(api->API_USER_LIST(aniListDisplayName));
  });

  userListWatcher.setFuture(userListJson);
}


void MainWindow::loadAnimeData() {
  progressBar->setValue(0);
  progressBar->setFormat("Loading anime data");
  progressBar->setMaximum(animeData.length());

  animeFutureData.setFuture(QtConcurrent::map(animeData, [&](Anime *& anime) {
    QString id = anime->getID();

    QJsonObject animeDataObject = api->get(api->API_ANIME(id));

    QJsonArray synArr = animeDataObject.value("synonyms").toArray();
    for(const QJsonValue &value : synArr) {
      anime->addSynonym(value.toString());
      knownAnime.append(value.toString());
    }

    QMetaObject::invokeMethod(this, "updateProgess", Qt::QueuedConnection);
  }));
}

void MainWindow::refreshList() {
  QJsonObject userListData = userListJson.result();

  userListData = userListData.value("lists").toObject();

  QStringList listNames = (QStringList() << "watching"
                                         << "completed"
                                         << "on_hold"
                                         << "dropped"
                                         << "plan_to_watch");

  QList<QTableWidget *> tableNames = (QList<QTableWidget *>()
                           << ui->currentlyWatchingTable
                           << ui->completedTable
                           << ui->onHoldTable
                           << ui->droppedTable
                           << ui->planToWatchTable);

  for(int i = 0; i < listNames.length(); i++) {
    for(QJsonValue ary : userListData.value(listNames.at(i)).toArray()) {
      QJsonObject anime = ary.toObject();

      int row = tableNames.at(i)->rowCount();

      QString title = anime.value("anime").toObject()
                           .value("title_romaji").toString();
      QTextDocument text; text.setHtml(title);

      knownAnime.append(title);
      knownAnime.append(anime.value("anime").toObject()
                        .value("title_japanese").toString());
      knownAnime.append(anime.value("anime").toObject()
                        .value("title_english").toString());

      QString plain_title = text.toPlainText();
      QString eps = QString::number(anime.value("episodes_watched").toInt());
      QString eptotal = QString::number(anime.value("anime").toObject()
                                             .value("total_episodes").toInt());
      QString score = QString::number(anime.value("score").toInt());
      QString type = anime.value("anime").toObject().value("type").toString();
      if (eptotal == "0") eptotal = "-";
      QString progress = QString(eps + " / " + eptotal);

      Anime *an = new Anime();
      an->setID(QString::number(anime.value("anime").toObject()
                                    .value("id").toInt()));

      animeData.append(an);

      QTableWidgetItem *titleData    = new QTableWidgetItem(plain_title);
      QTableWidgetItem *progressData = new QTableWidgetItem(progress);
      QTableWidgetItem *scoreData    = new QTableWidgetItem(score);
      QTableWidgetItem *typeData     = new QTableWidgetItem(type);

      tableNames.at(i)->insertRow(row);

      tableNames.at(i)->setItem(row, 0, titleData);
      tableNames.at(i)->setItem(row, 1, progressData);
      tableNames.at(i)->setItem(row, 2, scoreData);
      tableNames.at(i)->setItem(row, 3, typeData);

      progressBar->setValue((int)((i / (double)listNames.length()) * 100));
    }

    QString tab_title = listNames.at(i);
    QString tab_total = QString::number(tableNames.at(i)->rowCount());

    tab_title.replace(QString("_"), QString(" "));

    ui->listTabs->setTabText(i, tab_title + " (" + tab_total + ")");

    tableNames.at(i)->resizeColumnsToContents();
  }

  progressBar->setValue(0);
  progressBar->setFormat("");
  //loadAnimeData();
}
