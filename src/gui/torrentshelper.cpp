#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDesktopServices>
#include <regex>
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include "../lib/torrentrss.h"
#include "rulewizard.h"

void MainWindow::refreshTorrentListing() {
  ui->refreshButton->setText("Refresh (0)");
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
  checkForMatches();
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
  QDesktopServices::openUrl(ui->torrentTable->item(row, 5)->text());
}

void MainWindow::createRule(int row) {
  QString title = ui->torrentTable->item(row, 0)->text();
  QString sub   = ui->torrentTable->item(row, 2)->text();
  QString res   = ui->torrentTable->item(row, 3)->text();
  QString file  = ui->torrentTable->item(row, 4)->text();

  RuleWizard *rw = new RuleWizard(this, title, sub, res, file, default_rule);
  rw->show();

  connect(rw, SIGNAL(accepted()), SLOT(reloadRules()));
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

void MainWindow::reloadRules() {
  basic_rules.clear();
  adv_rules.clear();

  QDir rule_dir(QCoreApplication::applicationDirPath() + "/rules/");
  rule_dir.mkdir(".");
  rule_dir.setFilter(QDir::Files);
  for(int i = 0; i < rule_dir.entryList().count(); i++) {
    QString file_name = rule_dir.entryList().at(i);
    QFile file(rule_dir.absoluteFilePath(file_name));
    file.open(QFile::ReadOnly);
    QJsonObject json = QJsonDocument::fromJson(file.readAll()).object();

    if(json["rule_type"] == "advanced") {
      adv_rules.append(QRegExp(json["file_regex"].toString()));
    } else {
      QMap<QString, QString> values;
      values.insert("anime", json["anime_name"].toString());
      values.insert("subgroup", json["sub_group"].toString());
      values.insert("resolution", json["resolution"].toString());

      basic_rules.append(values);
    }
  }
}

void MainWindow::checkForMatches() {
  for(int j = 0; j < ui->torrentTable->rowCount(); j++) {
    QString title = ui->torrentTable->item(j, 0)->text();
    QString sub   = ui->torrentTable->item(j, 2)->text();
    QString res   = ui->torrentTable->item(j, 3)->text();
    QString file  = ui->torrentTable->item(j, 4)->text();

    for(int i = 0; i < basic_rules.length(); i++) {
      if(title == basic_rules.at(i).value("anime") && sub == basic_rules.at(i).value("subgroup") && res == basic_rules.at(i).value("resolution")) {
        verifyAndDownload(j);
      }
    }

    for(int i = 0; i < adv_rules.length(); i++) {
      if(adv_rules.at(i).exactMatch(file)) {
        verifyAndDownload(j);
      }
    }
  }
}

void MainWindow::verifyAndDownload(int row) {
  QString title = ui->torrentTable->item(row, 4)->text();
  QString file = title + ".dl";
  QDir history_dir(QCoreApplication::applicationDirPath() + "/history/");
  history_dir.mkdir(".");
  QFile f(history_dir.absoluteFilePath(file));
  if(!f.exists()) {
    download(row);
    f.open(QFile::WriteOnly);
    f.write("0");
  }

  trayIcon->showMessage("Shinjiru", title + " has started downloading.");
}
