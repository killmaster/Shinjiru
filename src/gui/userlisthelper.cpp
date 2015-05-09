/* Copyright 2015 Kazakuri */

#include <QHeaderView>
#include <QtConcurrent>
#include <QCheckBox>

#include "./ui_mainwindow.h"
#include "./mainwindow.h"

void MainWindow::loadUserList() {
  progress_bar->setValue(25);
  progress_bar->setFormat(tr("Downloading User List"));

  user_list_future = QtConcurrent::run([&]() {  // NOLINT
    User::sharedUser()->loadUserList();
  });

  async_registry.append(user_list_future);

  user_list_future_watcher.setFuture(user_list_future);
}

void MainWindow::userListLoaded() {
  progress_bar->setValue(30);
  progress_bar->setFormat(tr("Parsing User List"));

  airing.clear();
  airing_anime.clear();

  QLayoutItem *item;

  int index = 0;

  if (ui->listTabs->count() > 0) {
    index = ui->listTabs->currentIndex();
  }

  while ((item = layout->takeAt(0))) {
    delete item->widget();
    delete item;
  }

  while ((item = ui->scrollArea->widget()->layout()->takeAt(0)))
    delete item;

  int starting_value = progress_bar->value();
  int remaining_space = progress_bar->maximum() - starting_value;
  int space_per_list = remaining_space;
  int current_list = -1;
  QStringList list_order =
      settings->getValue(Settings::ListOrder, QStringList()).toStringList();

  for (QTableWidgetItem *i : tableWidgets) {
    delete i;
  }
  this->tableWidgets.clear();

  while (ui->listTabs->count()) {
    delete ui->listTabs->currentWidget();
  }

  ui->listTabs->clear();

  QMap<QString, QMap<QString, Anime*>> lists =
      User::sharedUser()->getUserList();

  if (lists.count() > 0)
    space_per_list = remaining_space / lists.count();

  for (QString key : lists.keys()) {
    if (key == "") continue;
    bool isCustom = true;

    if (key == "completed" || key == "on-hold" ||
        key == "plan_to_watch" || key == "dropped" ||
        key == "watching") {
      isCustom = false;
    }

    current_list++;
    QTableWidget *table = getListTable(isCustom);

    connect(table, SIGNAL(cellDoubleClicked(int, int)),
            SLOT(showAnimePanel(int, int)));

    QMap<QString, Anime*> list = lists.value(key);

    for (Anime *anime : list.values()) {
      QTableWidgetItem *titleData =
          new QTableWidgetItem(anime->getTitle());
      ProgressTableWidgetItem *progressData =
          new ProgressTableWidgetItem;
      QTableWidgetItem *scoreData = new QTableWidgetItem();
      FaceTableWidgetItem *faceData = new FaceTableWidgetItem;
      QTableWidgetItem *statusData;
      QTableWidgetItem  *typeData =
          new QTableWidgetItem(anime->getType());

      if (isCustom) {
        QString status = anime->getMyStatus();
        status.replace("plan to watch", "plan to Watch");
        status = status.at(0).toUpper() + status.right(status.length() - 1);

        statusData = new QTableWidgetItem(status);
        tableWidgets.append(statusData);
      }

      tableWidgets.append(titleData);
      tableWidgets.append(progressData);
      tableWidgets.append(scoreData);
      tableWidgets.append(faceData);
      tableWidgets.append(typeData);

      if (User::sharedUser()->scoreType() == 0 ||
          User::sharedUser()->scoreType() == 1) {
        scoreData->setData(Qt::DisplayRole,
                           anime->getMyScore().toInt());
      } else if (User::sharedUser()->scoreType() == 4) {
        scoreData->setData(Qt::DisplayRole,
                           anime->getMyScore().toDouble());
      } else if (User::sharedUser()->scoreType() == 3) {
        faceData->setText(anime->getMyScore());
      } else {
        scoreData->setText(anime->getMyScore());
      }

      if (anime->getAiringStatus() == "currently airing") {
        if (!airing.contains(anime->getTitle())) {
          airing << anime->getTitle();
          layout->addWidget(addAiring(anime));
        }
      }

      progressData->setText(QString::number(anime->getMyProgress()) +
                            " / " +
                            QString::number(anime->getEpisodeCount()));

      int row = table->rowCount();
      table->insertRow(row);

      table->setItem(row, 0, titleData);
      table->setItem(row, 1, progressData);
      if (User::sharedUser()->scoreType() == 3)
        table->setItem(row, 2, faceData);
      else
        table->setItem(row, 2, scoreData);
      if (isCustom) {
        table->setItem(row, 3, statusData);
        table->setItem(row, 4, typeData);
      } else {
        table->setItem(row, 3, typeData);
      }

      double current_progress = static_cast<double>(row) / list.count() *
          space_per_list;

      progress_bar->setValue(starting_value +
                             (current_list * space_per_list) +
                             static_cast<int>(current_progress));
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

    for (int i = 0; i < list_order.count(); i++) {
      if (list_order.at(i) == tab_title) {
        found = true;
        index = i;
      }
    }

    if (!found) list_order.append(tab_title);

    if (index == -1)
      ui->listTabs->addTab(page, tab_title + " (" + tab_total + ")");
    else
      ui->listTabs->insertTab(index, page,
                              tab_title + " (" + tab_total + ")");

    table->resizeColumnToContents(0);
    table->resizeColumnToContents(1);
    table->resizeColumnToContents(2);
    table->sortByColumn(0, Qt::SortOrder::AscendingOrder);

    QString rkey = tab_title + "State";
    rkey.replace(QRegExp("[ ]+"), "");
    rkey.replace(QRegExp("\\([0-9]+\\)"), "");

    QSettings s;
    QByteArray state = s.value(rkey).toByteArray();

    if (state != QByteArray()) {
      table->horizontalHeader()->restoreState(state);
    }
  }

  QTabBar *tb = ui->listTabs->tabBar();

  for (int i = 0; i < list_order.count(); i++) {
    QString current = list_order.at(i).toLower();
    QStringList tab_texts;
    for (int j = 0; j < ui->listTabs->count(); j++) {
      tab_texts << ui->listTabs->tabText(j).toLower();
    }

    int from = tab_texts.indexOf(QRegExp(QRegExp::escape(current) +
                                         " \\([0-9]+\\)"));
    tb->moveTab(from, i);
  }

  progress_bar->reset();
  progress_bar->setFormat("");
  over->removeDrawing("no anime");
  ui->listTabs->show();
  ui->listFilterLineEdit->show();

  if (User::sharedUser()->getAnimeList().count() == 0) {
    this->addNoAnimePrompt();
    this->showBrowseTab();
  }

  ui->actionRL->setEnabled(true);

  ui->listTabs->setCurrentIndex(index);

  settings->setValue(Settings::ListOrder, list_order);

  updateStatistics();
}

QTableWidget *MainWindow::getListTable(bool custom_list) {
  QTableWidget *table = new QTableWidget(this);

  QStringList default_list_labels;
  default_list_labels << tr("Title") << tr("Episodes") << tr("Score");

  if (custom_list) default_list_labels << tr("Status");

  default_list_labels << tr("Type");

  table->setColumnCount(custom_list ? 5 : 4);
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
  table->horizontalHeader()->setHighlightSections(false);

  connect(table, &QWidget::customContextMenuRequested,
          [=](QPoint pos) {  // NOLINT
    QTableWidgetItem *item = table->itemAt(pos);
    if (item == 0) return;
    int row = item->row();
    pos.setY(pos.y() + 120);

    QAction *pAnimePanel = new QAction(tr("Open Anime Panel"), table);
    QAction *pEpisodeIncrement =
        new QAction(tr("Increment Progress by 1"), table);
    QMenu   *pStatusUpdate = new QMenu(tr("Status"), table);
    QAction *pDeleteEntry = new QAction(tr("Delete Entry"), table);
    QMenu   *pCustomLists = new QMenu(tr("Custom Lists"), table);

    QAction *pWatching = new QAction(tr("Watching"), pStatusUpdate);
    QAction *pOnHold = new QAction(tr("On Hold"), pStatusUpdate);
    QAction *pPlanToWatch =
        new QAction(tr("Plan to Watch"), pStatusUpdate);
    QAction *pCompleted = new QAction(tr("Completed"), pStatusUpdate);
    QAction *pDropped = new QAction(tr("Dropped"), pStatusUpdate);

    QAction *pHideDefault =
        new QAction(tr("Hide Default"), pCustomLists);


    QString title = table->item(row, 0)->text();
    QString episodes = table->item(row, 1)->text();
    QString score = table->item(row, 2)->text();
    QString type = table->item(row, table->columnCount() - 1)->text();

    Anime *anime =
        User::sharedUser()->getAnimeByData(title, episodes, score, type);

    for (int i = 0; i < User::sharedUser()->customLists().length(); i++) {
      if (User::sharedUser()->customLists().at(i).toString().isEmpty())
        continue;

      QAction *temp = new QAction(
            User::sharedUser()->customLists().at(i).toString(),
            table);
      temp->setCheckable(true);

      if (anime->getCustomLists().at(i) == 1) {
        temp->setChecked(true);
      } else {
        temp->setChecked(false);
      }

      connect(temp, &QAction::toggled, [&, anime, i](bool selected) {  // NOLINT
        QList<int> custom = anime->getCustomLists();
        custom.replace(i, selected ? 1 : 0);
        anime->setCustomLists(custom);

        QMap<QString, QString> data;
        data.insert("id", anime->getID());
        QString d;
        for (int i = 0; i < custom.length(); i++) {
          d += QString::number(custom.at(i)) +
              ((i == custom.length() - 1) ? "" : ",");
        }

        data.insert("custom_lists", d);

        API::sharedAPI()->sharedAniListAPI()->put(
              API::sharedAPI()->sharedAniListAPI()->API_EDIT_LIST, data);

        this->userListLoaded();
      });

      pCustomLists->addAction(temp);
    }

    pHideDefault->setCheckable(true);
    pHideDefault->setChecked(anime->isDefaultHidden());

    pCustomLists->addAction(pHideDefault);

    connect(pHideDefault, &QAction::toggled,
            [&, anime, table](bool selected) {  // NOLINT
      QMap<QString, QString> data;
      data.insert("id", anime->getID());
      data.insert("hidden_default", selected ? "1" : "0");

      if (selected) {
        User::sharedUser()->removeFromList(anime->getMyStatus(), anime);
      }

      API::sharedAPI()->sharedAniListAPI()->put(
            API::sharedAPI()->sharedAniListAPI()->API_EDIT_LIST, data);

      this->userListLoaded();
    });

    pStatusUpdate->addAction(pWatching);
    pStatusUpdate->addAction(pOnHold);
    pStatusUpdate->addAction(pPlanToWatch);
    pStatusUpdate->addAction(pCompleted);
    pStatusUpdate->addAction(pDropped);

    connect(pAnimePanel, &QAction::triggered, [&, row, table]() {  // NOLINT
      this->showAnimePanel(row, 0, table);
    });

    connect(pEpisodeIncrement, &QAction::triggered,
            [&, row, table]() {  // NOLINT
      QString title = table->item(row, 0)->text();
      QString episodes = table->item(row, 1)->text();
      QString score = table->item(row, 2)->text();
      QString type = table->item(row, table->columnCount() - 1)->text();

      Anime *anime =
          User::sharedUser()->getAnimeByData(title, episodes, score, type);
      anime->setMyProgress(anime->getMyProgress() + 1);

      table->item(row, 1)->setText(QString::number(anime->getMyProgress()) +
                                   " / " +
                                   QString::number(anime->getEpisodeCount()));

      QMap<QString, QString> data;
      data.insert("id", anime->getID());
      data.insert("episodes_watched", QString::number(anime->getMyProgress()));

      API::sharedAPI()->sharedAniListAPI()->put(
            API::sharedAPI()->sharedAniListAPI()->API_EDIT_LIST, data);
    });

    connect(pWatching, &QAction::triggered, [&, row, table]() {  // NOLINT
      updateStatus(row, table, "watching");
    });

    connect(pPlanToWatch, &QAction::triggered, [&, row, table]() {  // NOLINT
      updateStatus(row, table, "plan to watch");
    });

    connect(pOnHold, &QAction::triggered, [&, row, table]() {  // NOLINT
      updateStatus(row, table, "on-hold");
    });

    connect(pDropped, &QAction::triggered, [&, row, table]() {  // NOLINT
     updateStatus(row, table, "dropped");
    });

    connect(pCompleted, &QAction::triggered, [&, row, table]() {  // NOLINT
      updateStatus(row, table, "completed");
    });

    connect(pDeleteEntry, &QAction::triggered, [&, row, table]() {  // NOLINT
      QString title = table->item(row, 0)->text();
      QString episodes = table->item(row, 1)->text();
      QString score = table->item(row, 2)->text();
      QString type = table->item(row, table->columnCount() - 1)->text();

      Anime *anime =
          User::sharedUser()->getAnimeByData(title, episodes, score, type);

      API::sharedAPI()->sharedAniListAPI()->deleteResource(
            API::sharedAPI()->sharedAniListAPI()->API_DELETE_ANIME(
              anime->getID()));

      User::sharedUser()->remove(anime);
      this->userListLoaded();
    });

    QMenu *pContextMenu = new QMenu(this);

    pContextMenu->addAction(pAnimePanel);
    pContextMenu->addAction(pEpisodeIncrement);
    pContextMenu->addMenu(pStatusUpdate);
    if (User::sharedUser()->customLists().length() > 0) {
      pContextMenu->addMenu(pCustomLists);
    } else {
      delete pCustomLists;
    }
    pContextMenu->addAction(pDeleteEntry);

    pContextMenu->exec(mapToGlobal(pos));

    delete pContextMenu;
    pContextMenu = nullptr;
  });

  return table;
}

AiringAnime *MainWindow::addAiring(Anime *anime) {
  if (anime->needsLoad() || anime->needsCover()) {
    User::sharedUser()->loadAnimeData(anime, true);
  }

  AiringAnime *newPanel =
      new AiringAnime(this, User::sharedUser()->scoreType());
  newPanel->setAnime(anime);


  int width = layout->geometry().width();
  int cwidth = layout->contentsWidth();

  if (ui->tabWidget->currentIndex() == 3)
    layout->setContentsMargins((width-cwidth)/2, 0, 0, 0);

  airing_anime.append(newPanel);

  return newPanel;
}

void MainWindow::filterList(int i) {
  Q_UNUSED(i)

  filterList(ui->listFilterLineEdit->text());
}

void MainWindow::filterList(QString filter) {
  if (ui->listTabs->currentWidget() == 0) return;
  if (ui->listTabs->currentWidget()->layout() == 0) return;
  if (ui->listTabs->currentWidget()->layout()->count() == 0) return;

  int visibleRows = 0;

  QTableWidget *w = static_cast<QTableWidget *>
      (ui->listTabs->currentWidget()->layout()->itemAt(0)->widget());

  if (w == 0) return;

  for (int i = 0; i < w->rowCount(); i++)
    w->hideRow(i);

  QList<QTableWidgetItem *> items = w->findItems(filter, Qt::MatchContains);

  for (int i = 0; i < items.count(); i++) {
    if (items.at(i) == 0) continue;
    if (items.at(i)->column() != 0 ) continue;

    w->showRow(items.at(i)->row());
    visibleRows++;
  }
  if (visibleRows == 0) {
    addSearchPrompt();
  } else {
    over->removeDrawing("blank_table");
    w->setAttribute(Qt::WA_TransparentForMouseEvents, false);
  }
}

void MainWindow::addSearchPrompt() {
  QTableWidget *w = static_cast<QTableWidget *>(
        ui->listTabs->currentWidget()->layout()->itemAt(0)->widget());

  QPoint location = w->mapTo(this, QPoint(0, 0));
  QPixmap *pix = new QPixmap(width(), height());
  pix->fill(Qt::transparent);
  QPainter p(pix);
  QRect r(location.x()+1, location.y()+24, w->width()-2, w->height()-25);
  p.fillRect(r, QColor(255, 255, 255));
  p.setPen(QColor(0, 0, 200));
  p.drawText(r, Qt::AlignCenter,
             tr("Nothing found, click here to open a search box."));
  over->addDrawing("blank_table", pix);

  w->setAttribute(Qt::WA_TransparentForMouseEvents);
}

void MainWindow::addNoAnimePrompt() {
  QTabWidget *w = static_cast<QTabWidget *>(ui->tabWidget);

  ui->listTabs->hide();
  ui->listFilterLineEdit->hide();

  QPoint location = w->mapTo(this, QPoint(0, 0));
  QPixmap *pix = new QPixmap(width(), height());
  pix->fill(Qt::transparent);
  QPainter p(pix);
  QRect r(location.x()+1, location.y()+34, w->width()-2, w->height()-35);
  p.setPen(QColor(0, 0, 0));
  QFont f = p.font();
  f.setPixelSize(20);
  p.setFont(f);
  p.drawText(r, Qt::AlignCenter,
             tr("Anime list empty, add some using the anime browse tab!"));
  over->addDrawing("no anime", pix);
  over->removeDrawing("blank_table");
}

void MainWindow::updateStatus(int row, QTableWidget *table, QString status) {
  QString title = table->item(row, 0)->text();
  QString episodes = table->item(row, 1)->text();
  QString score = table->item(row, 2)->text();
  QString type = table->item(row, table->columnCount() - 1)->text();

  Anime *anime =
      User::sharedUser()->getAnimeByData(title, episodes, score, type);

  QString old_status = anime->getMyStatus();
  anime->setMyStatus(status);

  QMap<QString, QString> data;
  data.insert("id", anime->getID());
  data.insert("list_status", anime->getMyStatus());

  API::sharedAPI()->sharedAniListAPI()->put(
        API::sharedAPI()->sharedAniListAPI()->API_EDIT_LIST, data);

  if (old_status != status) {
    User::sharedUser()->removeFromList(old_status, anime);
    this->userListLoaded();
  }
}
