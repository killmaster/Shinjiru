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

#include <string>
#include <regex>

#include "QtAwesome.h"
#include "app.h"
#include "anilist.h"
#include "torrents.h"

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

  ui->settingsButton->setIcon(awesome->icon(fa::gear,     black));
  ui->currentlyAiringButton->setIcon(awesome->icon(fa::clocko,   black));
  ui->torrentsButton->setIcon(awesome->icon(fa::rss,      black));
  ui->animeButton->setIcon(awesome->icon(fa::bars,     black));
  ui->statisticsButton->setIcon(awesome->icon(fa::piechart, black));

  QHeaderView *torrentHeader = ui->torrentTable->verticalHeader();
  torrentHeader->setDefaultSectionSize(torrentHeader->minimumSectionSize());

  readSettings();
  refreshAll();

  ui->tabWidget->tabBar()->hide();

  connect(ui->animeButton, SIGNAL(clicked()), SLOT(showAnimeTab()));
  connect(ui->settingsButton, SIGNAL(clicked()), SLOT(showSettingsTab()));
  connect(ui->torrentsButton, SIGNAL(clicked()), SLOT(showTorrentsTab()));
  connect(this, SIGNAL(logged_in()), SLOT(refreshAll()));
  connect(ui->usernameText, SIGNAL(textEdited(QString)), SLOT(enableApply()));
  connect(ui->passwordText, SIGNAL(textEdited(QString)), SLOT(enableApply()));
  connect(ui->applyButton, SIGNAL(clicked()), SLOT(applySettings()));
  connect(eventTimer, SIGNAL(timeout()), SLOT(tick()));
  connect(torrentRefreshTimer, SIGNAL(timeout()), SLOT(loadTorrents()));
  connect(ui->refreshButton, SIGNAL(clicked()), SLOT(loadTorrents()));

  ui->tabWidget->setCurrentIndex(0);

  if(aniListUsername != "" && aniListPassword != "") {
    if(AniList::login()) {
      emit logged_in();

      loadTorrents();
      ui->torrentTable->resizeColumnsToContents();
      ui->currentlyWatchingTable->resizeColumnsToContents();
    }
  }

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
  aniListUsername = settings.value("anilist/user", "").toString();
  aniListPassword = settings.value("anilist/pass", "").toString();
  int torrentRefreshInterval = settings.value("tinterval", 3600).toInt();

  ui->usernameText->setText(aniListUsername);
  ui->passwordText->setText(aniListPassword);

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

  settings.setValue("anilist/user", ui->usernameText->text());
  settings.setValue("anilist/pass", ui->passwordText->text());
  settings.setValue("tinterval", 3600 * 1000);
}

void MainWindow::refreshAll() {
  ui->displayName->setText(aniListDisplayName);

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

  if(aniListUsername != ui->usernameText->text() ||
    aniListPassword != ui->passwordText->text()) {
  }
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
    char* str = titles.at(i).toLocal8Bit().data();
    std::wstring name(str, str+strlen(str));
    std::basic_string<wchar_t> title = name.c_str();

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
}

void MainWindow::tick() {
  int remainingTime = torrentRefreshTimer->remainingTime() / 1000;
  ui->refreshButton->setText("Refresh (" +QString::number(remainingTime)+ ")");
  eventTimer->start(1000);
}
