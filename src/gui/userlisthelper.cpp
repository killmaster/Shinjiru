#include "ui_mainwindow.h"
#include "mainwindow.h"

#include <QHeaderView>
#include <QtConcurrent>

void MainWindow::loadUserList() {
  progress_bar->setValue(25);
  progress_bar->setFormat("Downloading User List");

  user_list_future = QtConcurrent::run([&]() {
    User::sharedUser()->loadUserList();
  });

  user_list_future_watcher.setFuture(user_list_future);

}

void MainWindow::userListLoaded() {
  progress_bar->setValue(30);
  progress_bar->setFormat("Parsing User List");

  airing.clear();

  QLayoutItem *item;

  while(item = layout->takeAt(0)){
    delete item->widget();
    delete item;
  }

  while((item = ui->scrollArea->widget()->layout()->takeAt(0)))
    delete item;

  int starting_value = progress_bar->value();
  int remaining_space = progress_bar->maximum() - starting_value;
  int space_per_list = remaining_space;
  int current_list = -1;

  while(ui->listTabs->count()) {
    delete ui->listTabs->widget(ui->listTabs->currentIndex());
  }

  ui->listTabs->clear();

  QMap<QString, QMap<QString, Anime*>> lists = User::sharedUser()->getUserList();

  if(lists.count() > 0)
    space_per_list = remaining_space / lists.count();

  for(QString key: lists.keys()) {
    if(key == "") continue;
    current_list++;
    QTableWidget *table = getListTable();

    connect(table, SIGNAL(cellDoubleClicked(int,int)), SLOT(showAnimePanel(int, int)));

    QMap<QString, Anime*> list = lists.value(key);

    for(Anime *anime : list.values()) {
      QTableWidgetItem        *titleData    = new QTableWidgetItem(anime->getTitle());
      ProgressTableWidgetItem *progressData = new ProgressTableWidgetItem;
      QTableWidgetItem        *scoreData    = new QTableWidgetItem();
      FaceTableWidgetItem     *faceData     = new FaceTableWidgetItem;
      QTableWidgetItem        *typeData     = new QTableWidgetItem(anime->getType());

      if(User::sharedUser()->scoreType() == 0 || User::sharedUser()->scoreType() == 1) {
        scoreData->setData(Qt::DisplayRole, anime->getMyScore().toInt());
      } else if(User::sharedUser()->scoreType() == 4) {
        scoreData->setData(Qt::DisplayRole, anime->getMyScore().toDouble());
      } else if(User::sharedUser()->scoreType() == 3) {
        faceData->setText(anime->getMyScore());
      }
      else {
        scoreData->setText(anime->getMyScore());
      }

      if(anime->getAiringStatus() == "currently airing") {
        if(!airing.contains(anime->getTitle())) {
          airing << anime->getTitle();
          layout->addWidget(addAiring(anime));
        }
      }

      progressData->setText(QString::number(anime->getMyProgress()) + " / " + QString::number(anime->getEpisodeCount()));

      int row = table->rowCount();
      table->insertRow(row);

      table->setItem(row, 0, titleData);
      table->setItem(row, 1, progressData);
      if(User::sharedUser()->scoreType() == 3)
        table->setItem(row, 2, faceData);
      else
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
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(table);
    page->setLayout(layout);

    bool found = false;
    int index = -1;

    for(int i = 0; i < ui->orderListWidget->count(); i++) {
      if(ui->orderListWidget->item(i)->text() == tab_title) {
        found = true;
        index = i;
      }
    }

    if(!found) ui->orderListWidget->addItem(tab_title);

    if(index == -1)
      ui->listTabs->addTab(page, tab_title + " (" + tab_total + ")");
    else
      ui->listTabs->insertTab(index, page, tab_title + " (" + tab_total + ")");

    table->resizeColumnToContents(0);
    table->resizeColumnToContents(1);
    table->resizeColumnToContents(2);
    table->sortByColumn(0, Qt::SortOrder::AscendingOrder);
  }

  QTabBar *tb = ui->listTabs->tabBar();

  for(int i = 0; i < ui->orderListWidget->count(); i++) {
    QString current = ui->orderListWidget->item(i)->text().toLower();
    QStringList tab_texts;
    for(int j = 0; j < ui->listTabs->count(); j++) {
      tab_texts << ui->listTabs->tabText(j).toLower();
    }
    int from = tab_texts.indexOf(QRegExp(current + " \\([0-9]+\\)"));
    tb->moveTab(from, i);
  }

  progress_bar->reset();
  progress_bar->setFormat("");

  updateStatistics();
}

QTableWidget *MainWindow::getListTable() {
  QTableWidget *table = new QTableWidget(this);

  QStringList default_list_labels;
  default_list_labels << tr("Title") << tr("Episodes") << tr("Score") << tr("Type");

  table->setColumnCount(4);
  table->setHorizontalHeaderLabels(default_list_labels);
  table->verticalHeader()->hide();
  table->setEditTriggers(QTableWidget::NoEditTriggers);
  table->setAlternatingRowColors(true);
  table->setSelectionMode(QTableWidget::SingleSelection);
  table->setContextMenuPolicy(Qt::CustomContextMenu);
  table->setSelectionBehavior(QTableWidget::SelectRows);
  table->verticalHeader()->setDefaultSectionSize(19);
  table->horizontalHeader()->setStretchLastSection(true);
  table->setSortingEnabled(true);

  connect(table, &QWidget::customContextMenuRequested, [=](QPoint pos) {
    QTableWidgetItem *item = table->itemAt(pos);
    if(item == 0) return;
    int row = item->row();
    pos.setY(pos.y() + 120);

    QAction *pAnimePanel       = new QAction("Open Anime Panel", table);
    QAction *pEpisodeIncrement = new QAction("Increment Progress by 1", table);
    QMenu   *pStatusUpdate     = new QMenu("Status", table);
    QAction *pDeleteEntry      = new QAction("Delete Entry", table);

    QAction *pWatching         = new QAction("Watching", pStatusUpdate);
    QAction *pOnHold           = new QAction("On Hold", pStatusUpdate);
    QAction *pPlanToWatch      = new QAction("Plan to Watch", pStatusUpdate);
    QAction *pCompleted        = new QAction("Completed", pStatusUpdate);
    QAction *pDropped          = new QAction("Dropped", pStatusUpdate);

    pStatusUpdate->setEnabled(false);
    pStatusUpdate->addAction(pWatching);
    pStatusUpdate->addAction(pOnHold);
    pStatusUpdate->addAction(pPlanToWatch);
    pStatusUpdate->addAction(pCompleted);
    pStatusUpdate->addAction(pDropped);

    pDeleteEntry->setEnabled(false);


    connect(pAnimePanel, &QAction::triggered, [&, row, table]() {
      this->showAnimePanel(row, 0, table);
    });

    connect(pEpisodeIncrement, &QAction::triggered, [&, row, table]() {
      QString title = table->item(row, 0)->text();
      QString episodes = table->item(row, 1)->text();
      QString score = table->item(row, 2)->text();
      QString type = table->item(row, 3)->text();

      Anime *anime = User::sharedUser()->getAnimeByData(title, episodes, score, type);
      anime->setMyProgress(anime->getMyProgress() + 1);


      table->item(row, 1)->setText(QString::number(anime->getMyProgress()) + " / " + QString::number(anime->getEpisodeCount()));

      QMap<QString, QString> data;
      data.insert("id", anime->getID());
      data.insert("episodes_watched", QString::number(anime->getMyProgress()));

      API::sharedAPI()->sharedAniListAPI()->put(API::sharedAPI()->sharedAniListAPI()->API_EDIT_LIST, data);
    });

    QMenu *pContextMenu = new QMenu(this);

    pContextMenu->addAction(pAnimePanel);
    pContextMenu->addAction(pEpisodeIncrement);
    pContextMenu->addMenu(pStatusUpdate);
    pContextMenu->addAction(pDeleteEntry);

    pContextMenu->exec(mapToGlobal(pos));

    delete pContextMenu;
    pContextMenu = nullptr;
  });

  return table;
}

AiringAnime *MainWindow::addAiring(Anime *anime) {
  if(anime->needsLoad() || anime->needsCover()) {
    User::sharedUser()->loadAnimeData(anime, true);
  }

  AiringAnime *newPanel = new AiringAnime(this, User::sharedUser()->scoreType());
  newPanel->setAnime(anime);

  int width = layout->geometry().width();
  int cwidth = layout->contentsWidth();

  if(ui->tabWidget->currentIndex() == 3)
    layout->setContentsMargins((width-cwidth)/2, 0, 0, 0);

  return newPanel;
}

void MainWindow::filterList(int) {
  filterList(ui->listFilterLineEdit->text());
}

void MainWindow::filterList(QString filter) {
  if(ui->listTabs->currentWidget() == 0) return;
  if(ui->listTabs->currentWidget()->layout() == 0) return;
  if(ui->listTabs->currentWidget()->layout()->count() == 0) return;

  int visibleRows = 0;

  QTableWidget *w = static_cast<QTableWidget *>(ui->listTabs->currentWidget()->layout()->itemAt(0)->widget());

  if(w == 0) return;

  for(int i = 0; i < w->rowCount(); i++)
    w->hideRow(i);

  QList<QTableWidgetItem *> items = w->findItems(filter, Qt::MatchContains);

  for(int i = 0; i < items.count(); i++) {
    if(items.at(i)->column() != 0 ) continue;

    w->showRow(items.at(i)->row());
    visibleRows++;
  }
  if(visibleRows == 0) {
    addSearchPrompt();
  } else {
    over->removeDrawing("blank_table");
    w->setAttribute(Qt::WA_TransparentForMouseEvents, false);
  }
}

void MainWindow::addSearchPrompt() {
  QTableWidget *w = static_cast<QTableWidget *>(ui->listTabs->currentWidget()->layout()->itemAt(0)->widget());

  QPoint location = w->mapTo(this, QPoint(0,0));
  QPixmap *pix = new QPixmap(width(), height());
  pix->fill(Qt::transparent);
  QPainter p(pix);
  QRect r(location.x()+1, location.y()+24, w->width()-2, w->height()-25);
  p.fillRect(r, QColor(255,255,255));
  p.setPen(QColor(0,0,200));
  p.drawText(r, Qt::AlignCenter, "Nothing found, click here to open a search box.");
  over->addDrawing("blank_table", pix);

  w->setAttribute(Qt::WA_TransparentForMouseEvents);
}
