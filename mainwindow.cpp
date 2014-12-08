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
#include <QHBoxLayout>
#include <string>
#include <regex>

#include "QtAwesome.h"
#include "app.h"
#include "torrents.h"
#include "anilistapi.h"
#include "anime.h"
#include "progresstablewidgetitem.h"
#include "animepanel.h"
#include "airinganime.h"
#include "flowlayout.h"

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
  animeImageCtrl      = nullptr;

  QFont font = ui->listTabs->tabBar()->font();
  font.setCapitalization(QFont::Capitalize);
  ui->listTabs->tabBar()->setFont(font);

  QWidget *container = new QWidget(ui->scrollArea);
  FlowLayout *layout = new FlowLayout(container);
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
  progressBar->setRange(0, 100);

  defaultListLabels << "Title" << "Episodes" << "Score" << "Type";

  readSettings();

  /*
   * Connect signals to slots
   */
  connect(this,                SIGNAL(displayNameAvailable()),             SLOT(loadList()));

  connect(ui->actionSettings,  SIGNAL(triggered()),                        SLOT(showSettingsTab()));
  connect(ui->applyButton,     SIGNAL(clicked()),                          SLOT(applySettings()));

  connect(ui->animeButton,     SIGNAL(clicked()),                          SLOT(showAnimeTab()));

  connect(ui->airingButton,    SIGNAL(clicked()),                          SLOT(showAiringTab()));

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
  connect(&animeDataWatcher,   SIGNAL(finished()),                         SLOT(processAnimeData()));

  connect(ui->actionRL,        SIGNAL(triggered()),                        SLOT(loadList()));
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
void MainWindow::showAiringTab()   { ui->tabWidget->setCurrentIndex(3); }

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

  QList<QTableWidgetItem *> items = ui->torrentTable->findItems(text, Qt::MatchContains);

  for(int i = 0; i < items.count(); i++) {
    if(items.at(i)->column() != 0 ) continue;
    bool show = true;

    if(checked) {
      QString f_title = items.at(i)->text();
      if(!airingTitles.contains(f_title)) show = false;
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

  if(aniListDisplayName.isEmpty()) return;

  userListJson = QtConcurrent::run([&]() {
    return api->get(api->API_USER_LIST(aniListDisplayName));
  });

  userListWatcher.setFuture(userListJson);
}

void MainWindow::refreshList() {
  while(ui->listTabs->count()) {
    delete ui->listTabs->widget(ui->listTabs->currentIndex());
  }
  ui->listTabs->clear();

  QJsonObject userListData = userListJson.result();

  userListData = userListData.value("lists").toObject();

  QStringList listNames = userListData.keys();
  int offset = 0;
  int index = 0;

  if((index = listNames.indexOf("watching")) != -1) {
    listNames.move(index, 0 - offset);
  } else offset++;

  if((index = listNames.indexOf("completed")) != -1) {
    listNames.move(index, 1 - offset);
  }else offset++;

  if((index = listNames.indexOf("on_hold")) != -1) {
    listNames.move(index, 2 - offset);
  }else offset++;

  if((index = listNames.indexOf("dropped")) != -1) {
    listNames.move(index, 3 - offset);
  }else offset++;

  if((index = listNames.indexOf("plan_to_watch")) != -1) {
    listNames.move(index, 4 - offset);
  }else offset++;

  for(int i = 0; i < listNames.length(); i++) {
    QTableWidget *table = new QTableWidget(this);

    table->setColumnCount(4);
    table->setHorizontalHeaderLabels(defaultListLabels);
    table->verticalHeader()->hide();
    table->setEditTriggers(QTableWidget::NoEditTriggers);
    table->setAlternatingRowColors(true);
    table->setSelectionMode(QTableWidget::SingleSelection);
    table->setSelectionBehavior(QTableWidget::SelectRows);
    table->verticalHeader()->setDefaultSectionSize(19);
    table->horizontalHeader()->setStretchLastSection(true);
    table->setSortingEnabled(true);

    connect(table, SIGNAL(cellDoubleClicked(int,int)), SLOT(showAnimePanel(int, int)));

    for(QJsonValue ary : userListData.value(listNames.at(i)).toArray()) {
      QJsonObject anime = ary.toObject();
      QJsonObject inner_anime = anime.value("anime").toObject();

      QString romaji_title  =                 inner_anime.value("title_romaji")    .toString();
      QString japan_title   =                 inner_anime.value("title_japanese")  .toString();
      QString eng_title     =                 inner_anime.value("title_english")   .toString();
      QString type          =                 inner_anime.value("type")            .toString();
      QString id            = QString::number(inner_anime.value("id")              .toInt());
      QString air_status    =                 inner_anime.value("airing_status")   .toString();
      int     i_eps_total   =                 inner_anime.value("total_episodes")  .toInt();
      QString i_avg_score   =                 inner_anime.value("average_score")   .toString();
      QUrl    image_url     =            QUrl(inner_anime.value("image_url_lge")   .toString());

      int     i_eps_watched =                 anime      .value("episodes_watched").toInt(0);
      QString notes         =                 anime      .value("notes")           .toString();
      int     i_rewatch     =                 anime      .value("rewatched")       .toInt(0);
      QString watch_status  =                 anime      .value("list_status")     .toString();


      QString score;
      int i_score;
      double f_score;

      if(score_type == 0 || score_type == 1) {
        i_score =  anime.value("score").toInt();
        score = QString::number(i_score);
      } else if(score_type == 4) {
        f_score = anime.value("score").toDouble();
        score = QString::number(f_score);
      } else {
        score = anime.value("score").toString();
      }

      QString eps_total     = QString::number(i_eps_total);
      QString eps_watched   = QString::number(i_eps_watched);

      romaji_title = QTextDocument(romaji_title).toPlainText().trimmed();
      japan_title  = QTextDocument(japan_title) .toPlainText().trimmed();
      eng_title    = QTextDocument(eng_title)   .toPlainText().trimmed();
      eps_total    = (eps_total == "0") ? "-" : eps_total;

      if(air_status == "currently airing") {
        airingTitles.insert(romaji_title);
        airingTitles.insert(japan_title);
        airingTitles.insert(eng_title);
      }

      Anime *newAnime = new Anime();
      newAnime->setRomajiTitle(romaji_title);
      newAnime->setJapaneseTitle(japan_title);
      newAnime->setEnglishTitle(eng_title);
      newAnime->setType(type);
      newAnime->setEpisodeCount(i_eps_total);
      newAnime->setID(id);
      newAnime->setAiringStatus(air_status);
      newAnime->setAverageScore(i_avg_score);
      newAnime->setCoverURL(image_url);
      newAnime->setMyProgress(i_eps_watched);
      newAnime->setMyScore(score);
      newAnime->setMyNotes(notes);
      newAnime->setMyRewatch(i_rewatch);
      newAnime->setMyStatus(watch_status);

      animeData.insert(romaji_title, newAnime);

      QTableWidgetItem        *titleData    = new QTableWidgetItem(romaji_title);
      ProgressTableWidgetItem *progressData = new ProgressTableWidgetItem;
      QTableWidgetItem        *scoreData    = new QTableWidgetItem();
      QTableWidgetItem        *typeData     = new QTableWidgetItem(type);

      if(score_type == 0 || score_type == 1) {
        scoreData->setData(Qt::DisplayRole, i_score);
      } else if(score_type == 4) {
        scoreData->setData(Qt::DisplayRole, f_score);
      }
      progressData->setText(eps_watched + " / " + eps_total);

      int row = table->rowCount();
      table->insertRow(row);

      table->setItem(row, 0, titleData);
      table->setItem(row, 1, progressData);
      table->setItem(row, 2, scoreData);
      table->setItem(row, 3, typeData);

      progressBar->setValue((int)((i / (double)listNames.length()) * 100));
    }

    QString tab_title = listNames.at(i);
    QString tab_total = QString::number(table->rowCount());

    tab_title.replace(QString("_"), QString(" "));

    QWidget *page = new QWidget(ui->listTabs);
    QHBoxLayout *layout = new QHBoxLayout(ui->listTabs);
    layout->addWidget(table);
    page->setLayout(layout);

    ui->listTabs->addTab(page, tab_title + " (" + tab_total + ")");

    table->resizeColumnToContents(0);
    table->resizeColumnToContents(1);
    table->resizeColumnToContents(2);
  }

  progressBar->setValue(0);
  progressBar->setFormat("");

  loadAiring();
}

void MainWindow::showAnimePanel(int row, int column) {
  Q_UNUSED(column);

  QTableWidget *source = static_cast<QTableWidget *>(sender());
  QString title = source->item(row, 0)->text();

  AnimePanel *ap = new AnimePanel(this, animeData[title.toUtf8().data()]);

  if(ap->loadNeeded) {
    loadAnimeData(animeData[title.toUtf8().data()]->getID(), animeData[title.toUtf8().data()]->getRomajiTitle());
  }
  ap->show();
}

void MainWindow::loadAnimeData(QString ID, QString name) {
  loadingData = true;
  if(ID.isEmpty()) return;

  userAnimeData = QtConcurrent::run([&]() {
    return api->get(api->API_ANIME(ID));
  });

  if(animeImageCtrl != nullptr && !loadingData) delete animeImageCtrl;
  animeImageCtrl = new FileDownloader(animeData[name]->getCoverURL(), this);
  connect(animeImageCtrl, &FileDownloader::downloaded, [&, name] () {
    animeData[name]->setCoverImageData(animeImageCtrl->downloadedData());
    loadingData = false;
  });

  animeDataWatcher.setFuture(userAnimeData);
}

void MainWindow::processAnimeData() {
  QJsonObject result = userAnimeData.result();

  QJsonArray synonyms = result.value("synonyms").toArray();
  QString title       = result.value("title_romaji").toString().trimmed();
  QString description = result.value("description").toString();

  Anime *anime = animeData.value(title);

  for(int i = 0; i < synonyms.count(); i++) {
    anime->addSynonym(synonyms.at(i).toString());
  }

  anime->setSynopsis(description);

  anime->finishReload();
}

int MainWindow::scoreType() { return score_type; }

void MainWindow::loadAiring() {
  QSetIterator<QString> i(airingTitles);
  while (i.hasNext()) {
    AiringAnime *newPanel = new AiringAnime(this);
    Anime *anime = animeData.value(i.next());
    if(anime == 0) continue;
    newPanel->setAnime(anime);

    if(anime->needsLoad()) {
      loadAnimeData(anime->getID(), anime->getRomajiTitle());
      while(loadingData) QCoreApplication::processEvents();
    }
    ui->scrollArea->widget()->layout()->addWidget(newPanel);
  }
}
