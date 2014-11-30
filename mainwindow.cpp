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
#include <QInputDialog>

#include <string>
#include <regex>

#include "QtAwesome.h"
#include "app.h"
#include "torrents.h"
#include "anilistapi.h"

#include "anitomy/anitomy/anitomy.h"


MainWindow::MainWindow(QWidget *parent):
  QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  QFile styleFile(":/style.css");
  styleFile.open(QFile::ReadOnly);
  QString styleSheet = QLatin1String(styleFile.readAll());
  qApp->setStyleSheet(styleSheet);

  awesome = new QtAwesome(qApp);
  awesome->initFontAwesome();

  torrentRefreshTimer = new QTimer(this);
  eventTimer = new QTimer(this);

  QVariantMap black;
  black.insert("color", QColor(0, 0, 0));
  black.insert("color-active", QColor(0, 0, 0));
  black.insert("color-disabled", QColor(0, 0, 0));
  black.insert("color-selected", QColor(0, 0, 0));

  ui->currentlyAiringButton->setIcon(awesome->icon(fa::clocko, black));
  ui->torrentsButton->setIcon(awesome->icon(fa::rss, black));
  ui->animeButton->setIcon(awesome->icon(fa::bars, black));
  ui->statisticsButton->setIcon(awesome->icon(fa::piechart, black));

  QHeaderView *torrentHeader = ui->torrentTable->verticalHeader();
  torrentHeader->setDefaultSectionSize(torrentHeader->minimumSectionSize());

  readSettings();
  refreshAll();

  ui->tabWidget->tabBar()->hide();

  connect(ui->animeButton, SIGNAL(clicked()), SLOT(showAnimeTab()));
  connect(ui->actionSettings, SIGNAL(triggered()), SLOT(showSettingsTab()));
  connect(ui->torrentsButton, SIGNAL(clicked()), SLOT(showTorrentsTab()));
  connect(this, SIGNAL(logged_in()), SLOT(refreshAll()));
  connect(ui->usernameText, SIGNAL(textEdited(QString)), SLOT(enableApply()));
  connect(ui->passwordText, SIGNAL(textEdited(QString)), SLOT(enableApply()));
  connect(ui->applyButton, SIGNAL(clicked()), SLOT(applySettings()));
  connect(eventTimer, SIGNAL(timeout()), SLOT(tick()));
  connect(torrentRefreshTimer, SIGNAL(timeout()), SLOT(loadTorrents()));
  connect(ui->refreshButton, SIGNAL(clicked()), SLOT(loadTorrents()));
  connect(ui->torrentFilter, SIGNAL(textChanged(QString)),
          SLOT(filterTorrents(QString)));

  connect(ui->torrentTable,SIGNAL(customContextMenuRequested(QPoint)),
          SLOT(torrentContextMenu(QPoint)));
  connect(&userWatcher, SIGNAL(finished()), SLOT(refreshUser()));
  connect(&userListWatcher, SIGNAL(finished()), SLOT(refreshList()));
  connect(this, SIGNAL(displayNameAvailable()), SLOT(loadList()));

  ui->tabWidget->setCurrentIndex(0);

  api = new AniListAPI(this, "Shinjiru", "");

  if(api->init() == AniListAPI::OK) {
    emit logged_in();

    loadUser();
    ui->currentlyWatchingTable->resizeColumnsToContents();
  } else {
    bool ok;
    QString message = "Authorization code:                                                                                    ";
    QString text = QInputDialog::getText(this, tr("Authorization Code Request"),
                                         tr(message.toLocal8Bit().data()),
                                         QLineEdit::Normal, "", &ok);
    if (ok && !text.isEmpty())
      api->setAuthorizationCode(text);

    if(api->init() != AniListAPI::OK) exit(8);

    emit logged_in();

    loadUser();
    ui->currentlyWatchingTable->resizeColumnsToContents();
  }

  loadTorrents();
  ui->torrentTable->resizeColumnsToContents();

  eventTimer->start(1000);
}

MainWindow::~MainWindow() {
  delete ui;
  delete awesome;
}

void MainWindow::closeEvent(QCloseEvent *event) {
  writeSettings();
  event->accept();
}

void MainWindow::readSettings() {
  QSettings settings;

  QPoint pos = settings.value("window/pos", QPoint(200, 200)).toPoint();
  QSize size = settings.value("window/size", QSize(800, 600)).toSize();
  bool wasMaximized = settings.value("window/maximized", false).toBool();
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

void MainWindow::refreshAll() {
  ui->displayName->setText("");

  QImage userImage;
  ui->userImage->setPixmap(QPixmap::fromImage(userImage));
}

void MainWindow::showAnimeTab() {
  ui->tabWidget->setCurrentIndex(0);
}

void MainWindow::showSettingsTab() {
  ui->tabWidget->setCurrentIndex(1);
}

void MainWindow::showTorrentsTab() {
  ui->tabWidget->setCurrentIndex(2);
}

void MainWindow::enableApply() {
  ui->applyButton->setEnabled(true);
}

void MainWindow::applySettings() {
  writeSettings();
}

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

    QTableWidgetItem *titleItem = new QTableWidgetItem(QString(parsedTitle));
    QTableWidgetItem *epItem = new QTableWidgetItem(QString(episodeNumber));
    QTableWidgetItem *subItem = new QTableWidgetItem(QString(subGroup));
    QTableWidgetItem *videoItem = new QTableWidgetItem(QString(videoType));
    QTableWidgetItem *linkItem = new QTableWidgetItem(QString(links.at(i)));

    if(episodeNumber == "") {
      qDebug() << "Unknown episode for: " << parsedTitle << ", skipping";
      offset++;
      continue;
    }

    ui->torrentTable->setItem(i - offset, 0, titleItem);
    ui->torrentTable->setItem(i - offset, 1, epItem);
    ui->torrentTable->setItem(i - offset, 2, subItem);
    ui->torrentTable->setItem(i - offset, 3, videoItem);
    ui->torrentTable->setItem(i - offset, 4, linkItem);
  }

  while(offset > 0) {
      ui->torrentTable->removeRow(ui->torrentTable->rowCount() - 1);
      offset--;
  }

  torrentRefreshTimer->start();
  filterTorrents(ui->torrentFilter->text());
}

void MainWindow::tick() {
  int remainingTime = torrentRefreshTimer->remainingTime() / 1000;
  ui->refreshButton->setText("Refresh (" +QString::number(remainingTime)+ ")");
  eventTimer->start(1000);
}

void MainWindow::filterTorrents(QString text) {
  for(int i = 0; i < ui->torrentTable->rowCount(); i++)
    ui->torrentTable->hideRow(i);

  QList<QTableWidgetItem *> items =
    ui->torrentTable->findItems(text, Qt::MatchContains);

  for(int i = 0; i < items.count(); i++) {
    if(items.at(i)->column() != 0 ) continue;
    ui->torrentTable->showRow(items.at(i)->row());
  }
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
  ui->displayName->setText(userData.value("display_name").toString());
  emit displayNameAvailable();
  QUrl imageUrl(userData.value("image_url_med").toString());
  userImageCtrl = new FileDownloader(imageUrl, this);
  connect(userImageCtrl, SIGNAL(downloaded()), SLOT(setUserImage()));
}

void MainWindow::setUserImage() {
  QPixmap u_image;
  u_image.loadFromData(userImageCtrl->downloadedData());
  ui->userImage->setPixmap(u_image);
}

void MainWindow::loadList() {
  userListJson = QtConcurrent::run([&]() {
    return api->get(api->API_USER_LIST(ui->displayName->text()));
  });

  userListWatcher.setFuture(userListJson);
}

void MainWindow::refreshList() {
  QJsonObject userListData = userListJson.result();
  QJsonDocument doc(userListData);
  QByteArray bytes = doc.toJson();

  qDebug() << bytes;
}
