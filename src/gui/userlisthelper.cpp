#include "ui_mainwindow.h"
#include "mainwindow.h"

#include <QHeaderView>
#include <QtConcurrent>

#include "progresstablewidgetitem.h"

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

  ui->listTabs->clear();

  QMap<QString, QMap<QString, Anime*>> lists = user->getUserList();

  space_per_list = remaining_space / lists.count();

  for(QString key: lists.keys()) {
    current_list++;
    QTableWidget *table = getListTable();

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

      if(anime->getAiringStatus() == "currently airing") {
        ui->scrollArea->widget()->layout()->addWidget(addAiring(anime));
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

QTableWidget *MainWindow::getListTable() {
  QTableWidget *table = new QTableWidget(this);

  QStringList default_list_labels;
  default_list_labels << "Title" << "Episodes" << "Score" << "Type";

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

  return table;
}

AiringAnime *MainWindow::addAiring(Anime *anime) {
  if(anime->needsLoad()) {
    QtConcurrent::run([&, anime]() {
      user->loadAnimeData(anime);
    });
  }

  AiringAnime *newPanel = new AiringAnime(this);
  newPanel->setAnime(anime);
  return newPanel;
}