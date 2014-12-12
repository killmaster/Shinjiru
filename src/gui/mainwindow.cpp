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
#include "anilistapi.h"
#include "../app.h"
#include "../lib/torrentrss.h"
#include "../api/anime.h"
#include "../api/api.h"

#include "progresstablewidgetitem.h"
#include "animepanel.h"
#include "airinganime.h"
#include "flowlayout.h"

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

  awesome->initFontAwesome();

  event_timer         = new QTimer(this);
  progress_bar        = new QProgressBar(ui->statusBar);

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

  connect(ui->actionExit,     &QAction::triggered, []() {exit(0);});
  connect(ui->actionAbout,    &QAction::triggered, []() {return;});
  connect(ui->actionHelp,     &QAction::triggered, []() {return;});
  connect(ui->actionRL,       &QAction::triggered, [&]() {loadUserList();});
  connect(ui->actionVAL,      SIGNAL(triggered()), SLOT(viewAnimeList()));
  connect(ui->actionVD,       SIGNAL(triggered()), SLOT(viewDashboard()));
  connect(ui->actionVP,       SIGNAL(triggered()), SLOT(viewProfile()));

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
}

void MainWindow::closeEvent(QCloseEvent *event) {
  settings->write_settings();
  event->accept();
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

void MainWindow::viewDashboard() { QDesktopServices::openUrl(QString("http://anilist.co/home")); }
void MainWindow::viewProfile()   { QDesktopServices::openUrl(QString("http://anilist.co/user/") + user->displayName()); }
void MainWindow::viewAnimeList() { QDesktopServices::openUrl(QString("http://anilist.co/animelist/") + user->displayName()); }

void MainWindow::loadUser() {
  progress_bar->setFormat("Loading User");
  progress_bar->setValue(15);
  user_future = QtConcurrent::run([&]() {
    user = new User(api);
    return user;
  });

  user_future_watcher.setFuture(user_future);
}

void MainWindow::userLoaded() {
  progress_bar->setValue(20);
  progress_bar->setFormat("User Loaded");

  connect(user, SIGNAL(new_image()), SLOT(repaint()));
  repaint();

  loadUserList();
}

void MainWindow::loadUserList() {
  progress_bar->setValue(25);
  progress_bar->setFormat("Downloading User List");

  user_list_future = QtConcurrent::run([&]() {
    user->loadUserList();
    return user;
  });

  user_list_future_watcher.setFuture(user_list_future);

}

void MainWindow::userListLoaded() {
  progress_bar->setValue(30);
  progress_bar->setFormat("Parsing User List");

  int starting_value = progress_bar->value();
  int remaining_space = progress_bar->maximum() - starting_value;
  int space_per_list = remaining_space;
  int current_list = -1;

  while(ui->listTabs->count()) {
    delete ui->listTabs->widget(ui->listTabs->currentIndex());
  }

  QStringList default_list_labels;
  default_list_labels << "Title" << "Episodes" << "Score" << "Type";

  ui->listTabs->clear();

  QMap<QString, QMap<QString, Anime*>> lists = user->getUserList();

  space_per_list = remaining_space / lists.count();

  for(QString key: lists.keys()) {
    current_list++;
    QTableWidget *table = new QTableWidget(this);

    table->setColumnCount(4);
    table->setHorizontalHeaderLabels(default_list_labels);
    table->verticalHeader()->hide();
    table->setEditTriggers(QTableWidget::NoEditTriggers);
    table->setAlternatingRowColors(true);
    table->setSelectionMode(QTableWidget::SingleSelection);
    table->setSelectionBehavior(QTableWidget::SelectRows);
    table->verticalHeader()->setDefaultSectionSize(19);
    table->horizontalHeader()->setStretchLastSection(true);
    table->setSortingEnabled(true);

    connect(table, SIGNAL(cellDoubleClicked(int,int)), SLOT(showAnimePanel(int, int)));

    QMap<QString, Anime*> list = lists.value(key);

    for(Anime *anime : list.values()) {
      QTableWidgetItem        *titleData    = new QTableWidgetItem(anime->getRomajiTitle());
      ProgressTableWidgetItem *progressData = new ProgressTableWidgetItem;
      QTableWidgetItem        *scoreData    = new QTableWidgetItem();
      QTableWidgetItem        *typeData     = new QTableWidgetItem(anime->getType());

      if(user->scoreType() == 0 || user->scoreType() == 1) {
        scoreData->setData(Qt::DisplayRole, anime->getMyScore().toInt());
      } else if(user->scoreType() == 4) {
        scoreData->setData(Qt::DisplayRole, anime->getMyScore().toDouble());
      } else {
        scoreData->setText(anime->getMyScore());
      }

      progressData->setText(QString::number(anime->getMyProgress()) + " / " + QString::number(anime->getEpisodeCount()));

      int row = table->rowCount();
      table->insertRow(row);

      table->setItem(row, 0, titleData);
      table->setItem(row, 1, progressData);
      table->setItem(row, 2, scoreData);
      table->setItem(row, 3, typeData);

      double current_progress = (double)row / list.count() * space_per_list;

      progress_bar->setValue(starting_value + (current_list * space_per_list) + (int)current_progress);
    }

    QString tab_title = key;
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
    table->sortByColumn(0, Qt::SortOrder::AscendingOrder);
  }

  progress_bar->setValue(0);
  progress_bar->setFormat("");
}

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
