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
#include "rulemanager.h"

void MainWindow::refreshTorrentListing() {
  ui->refreshButton->setText("Refresh (0)");
  torrent_refresh_time = torrent_interval;

  TorrentRSS *torrents = new TorrentRSS(0);
  QEventLoop rssLoop;
  connect(torrents, SIGNAL(done()), &rssLoop, SLOT(quit()));
  torrents->fetch();
  rssLoop.exec();

  QStringList titles = QStringList(*(torrents->getTitles()));
  QStringList links = QStringList(*(torrents->getLinks()));
  int offset = 0;

  delete torrents;

  for(int i = 0; i < titles.length(); i++) {
    if(ui->torrentTable->rowCount() <= i)
      ui->torrentTable->insertRow(i);

    QMap<QString, QString> result;

    try {
      result = anitomy->parse(titles.at(i));
    } catch(std::regex_error& e) {
      Q_UNUSED(e);
      qWarning() << "Error parsing: " << titles.at(i);
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
      offset++;
      continue;
    }

    if(links.at(i) == "") {
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

  ui->torrentTable->resizeColumnsToContents();
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
  download_count++;
  count_total++;

  ui->labelDownloadedTotal->setText(QString::number(count_total));
  ui->labelDownloadedLaunch->setText(QString::number(download_count));

  settings->setValue(Settings::DownloadCount, count_total);
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
      Anime *filter_anime = User::sharedUser()->getAnimeByTitle(f_title);
      if(filter_anime == 0) continue;
      if(filter_anime->getAiringStatus() != "currently airing") show = false;
      if(filter_anime->getMyStatus() != "watching" && filter_anime->getMyStatus() != "plan to watch") show = false;
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

    QDate expires = QDate::currentDate();
    expires = expires.addDays(7 * json["expires"].toInt());

    if(QDate::currentDate().daysTo(expires) < 0) {
      file.remove();
      continue;
    }

    if(json["rule_type"] == "advanced") {
      QMap<QString, QVariant> values;
      values.insert("regexp", QRegExp(json["file_regex"].toString()));
      values.insert("expires", expires);
      adv_rules.append(values);
    } else {
      QMap<QString, QVariant> values;
      values.insert("anime", json["anime_name"].toString());
      values.insert("subgroup", json["sub_group"].toString());
      values.insert("resolution", json["resolution"].toString());
      values.insert("expires", expires);

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
      if(QDate::currentDate().daysTo(basic_rules.at(i).value("expires").toDate()) < 0) {
        basic_rules.removeAt(i);
        continue;
      }
      if(title == basic_rules.at(i).value("anime").toString() && sub == basic_rules.at(i).value("subgroup").toString() && res == basic_rules.at(i).value("resolution").toString()) {
        verifyAndDownload(j);
      }
    }

    for(int i = 0; i < adv_rules.length(); i++) {
      if(QDate::currentDate().daysTo(adv_rules.at(i).value("expires").toDate()) < 0) {
        adv_rules.removeAt(i);
        continue;
      }

      if(adv_rules.at(i).value("regexp").toRegExp().exactMatch(file)) {
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
    download_rule++;
    rule_total++;

    ui->labelRulesLaunch->setText(QString::number(download_rule));
    ui->labelRulesTotal->setText(QString::number(rule_total));

    settings->setValue(Settings::RuleCount, rule_total);

    trayIcon->showMessage("Shinjiru", title + tr(" has started downloading."));
    f.open(QFile::WriteOnly);
    f.write("0");
  }
}

void MainWindow::showRuleManager() {
  RuleManager *rm = new RuleManager(this, default_rule);
  rm->show();

  connect(rm, SIGNAL(accepted()), SLOT(reloadRules()));
}
