/* Copyright 2015 Kazakuri */

#include <QDesktopServices>
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>

#include <regex> //NOLINT

#include "./mainwindow.h"
#include "./ui_mainwindow.h"
#include "../lib/torrentrss.h"
#include "./settingsdialog.h"

void MainWindow::refreshTorrentListing() {
  qDebug() << "Refreshing torrent listing..."
           << "(" + QString::number(torrent_refresh_time) + ")";

  ui->refreshButton->setText("Refresh (0)");
  torrent_refresh_time =
      settings->getValue(Settings::TorrentRefreshTime, 900).toInt();

  TorrentRSS *torrents = new TorrentRSS(0);
  QEventLoop rssLoop;
  connect(torrents, SIGNAL(done()), &rssLoop, SLOT(quit()));
  torrents->fetch();
  rssLoop.exec();

  for (QTableWidgetItem *i : torrentWidgets) {
    delete i;
  }

  torrentWidgets.clear();

  QStringList titles = torrents->getTitles();
  QStringList links = torrents->getLinks();
  int offset = 0;

  for (int i = 0; i < titles.length(); i++) {
    if (ui->torrentTable->rowCount() <= i)
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

    if (episodeNumber == "") {
      offset++;
      continue;
    }

    if (links.at(i) == "") {
      offset++;
      continue;
    }

    QTableWidgetItem *titleItem = new QTableWidgetItem(parsedTitle);
    QTableWidgetItem *epItem = new QTableWidgetItem(episodeNumber);
    QTableWidgetItem *subItem = new QTableWidgetItem(subGroup);
    QTableWidgetItem *videoItem = new QTableWidgetItem(videoType);
    QTableWidgetItem *fileNameItem = new QTableWidgetItem(titles.at(i));
    QTableWidgetItem *linkItem = new QTableWidgetItem(links.at(i));

    torrentWidgets.append(titleItem);
    torrentWidgets.append(epItem);
    torrentWidgets.append(subItem);
    torrentWidgets.append(videoItem);
    torrentWidgets.append(fileNameItem);
    torrentWidgets.append(linkItem);

    ui->torrentTable->setItem(i - offset, 0, titleItem);
    ui->torrentTable->setItem(i - offset, 1, epItem);
    ui->torrentTable->setItem(i - offset, 2, subItem);
    ui->torrentTable->setItem(i - offset, 3, videoItem);
    ui->torrentTable->setItem(i - offset, 4, fileNameItem);
    ui->torrentTable->setItem(i - offset, 5, linkItem);
  }

  while (offset > 0) {
    ui->torrentTable->removeRow(ui->torrentTable->rowCount() - 1);
    offset--;
  }

  torrents->deleteLater();
  torrents = nullptr;
  qDebug() << ui->torrentTable->rowCount() << "torrents loaded";

  ui->torrentTable->resizeColumnsToContents();
  filterTorrents(ui->torrentFilter->text(), ui->chkHideUnknown->isChecked());
  checkForMatches();
}

void MainWindow::torrentContextMenu(QPoint pos) {
  QTableWidgetItem *item = ui->torrentTable->itemAt(pos);
  if (item == 0) return;

  int row = item->row();
  pos.setY(pos.y() + 120);
  QAction *pDownloadAction = new QAction("Download", ui->torrentTable);
  QAction *pRuleAction = new QAction("Create rule", ui->torrentTable);

  QSignalMapper *signalMapper1 = new QSignalMapper(this);
  QSignalMapper *signalMapper2 = new QSignalMapper(this);

  signalMapper1->setMapping(pDownloadAction, row);
  signalMapper2->setMapping(pRuleAction, row);

  connect(pDownloadAction, SIGNAL(triggered()), signalMapper1, SLOT(map()));
  connect(signalMapper1, SIGNAL(mapped(int)), this, SLOT(download(int)));

  connect(pRuleAction, SIGNAL(triggered()), signalMapper2, SLOT(map()));
  connect(signalMapper2, SIGNAL(mapped(int)), this, SLOT(createRule(int)));

  QMenu *pContextMenu = new QMenu( this);
  pContextMenu->addAction(pDownloadAction);
  pContextMenu->addAction(pRuleAction);
  pContextMenu->exec(mapToGlobal(pos));
}

void MainWindow::download(int row) {
  QDesktopServices::openUrl(ui->torrentTable->item(row, 5)->text());
  download_count++;
  count_total++;

  qDebug() << "Downloading" << ui->torrentTable->item(row, 5)->text();

  ui->labelDownloadedTotal->setText(QString::number(count_total));
  ui->labelDownloadedLaunch->setText(QString::number(download_count));

  settings->setValue(Settings::DownloadCount, count_total);
}

void MainWindow::createRule(int row) {
  QString title = ui->torrentTable->item(row, 0)->text();
  QString sub   = ui->torrentTable->item(row, 2)->text();
  QString res   = ui->torrentTable->item(row, 3)->text();
  QString file  = ui->torrentTable->item(row, 4)->text();

  SettingsDialog *s = new SettingsDialog(this);
  s->showTorrentRules(title, sub, res, file);

  connect(s, &QDialog::accepted, [&] () {  // NOLINT
    toggleAnimeRecognition(
          settings->getValue(Settings::AnimeRecognitionEnabled, true).toBool());

    torrents_enabled =
        settings->getValue(Settings::TorrentsEnabled, true).toBool();

    reloadSmartTitles();
    reloadRules();
  });
}


void MainWindow::filterTorrents(QString text, bool checked) {
  for (int i = 0; i < ui->torrentTable->rowCount(); i++)
    ui->torrentTable->hideRow(i);

  QList<QTableWidgetItem *> items =
      ui->torrentTable->findItems(text, Qt::MatchContains);

  for (int i = 0; i < items.count(); i++) {
    if (items.at(i)->column() != 0 ) continue;
    bool show = true;

    if (checked) {
      QString f_title = items.at(i)->text();
      Anime *filter_anime = User::sharedUser()->getAnimeByTitle(f_title);
      if (filter_anime == 0) continue;
      if (filter_anime->getAiringStatus() != "currently airing") show = false;
      if (filter_anime->getMyStatus() != "watching" &&
          filter_anime->getMyStatus() != "plan to watch") show = false;
    }

    if (show)
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
  rule_dir.setFilter(QDir::Files);

  QFile tor_rule_file(QCoreApplication::applicationDirPath() + "/rules.json");
  tor_rule_file.open(QFile::ReadWrite);
  QJsonObject torrent_rules =
      QJsonDocument::fromJson(tor_rule_file.readAll()).object();

  if (rule_dir.exists()) {
    for (int i = 0; i < rule_dir.entryList().count(); i++) {
      QJsonObject rule;
      QString file_name = rule_dir.entryList().at(i);
      QFile file(rule_dir.absoluteFilePath(file_name));
      file.open(QFile::ReadOnly);
      QJsonObject json = QJsonDocument::fromJson(file.readAll()).object();

      QDate expires = QDate::currentDate();
      expires = expires.addDays(7 * json["expires"].toInt());

      if (QDate::currentDate().daysTo(expires) < 0) {
        file.remove();
        continue;
      }

      QString title;

      if (json["rule_type"] == "advanced") {
        QMap<QString, QVariant> values;
        values.insert("regexp", QRegExp(json["file_regex"].toString()));
        values.insert("expires", expires);
        adv_rules.append(values);

        rule.insert("rule_type", json["rule_type"]);
        rule.insert("regexp", json["file_regex"].toString());
        rule.insert("expires", expires.toString());

        title = rule.value("regexp").toString();
      } else {
        QMap<QString, QVariant> values;
        values.insert("anime", json["anime_name"].toString());
        values.insert("subgroup", json["sub_group"].toString());
        values.insert("resolution", json["resolution"].toString());
        values.insert("expires", expires);

        rule.insert("rule_type", json["rule_type"]);
        rule.insert("anime", json["anime_name"].toString());
        rule.insert("subgroup", json["sub_group"].toString());
        rule.insert("resolution", json["resolution"].toString());
        rule.insert("expires", expires.toString());

        title = rule.value("anime").toString();
        basic_rules.append(values);
      }

      file.remove();
      torrent_rules.insert(title, rule);
    }

    rule_dir.removeRecursively();
    tor_rule_file.write(QJsonDocument(torrent_rules).toJson());
  }

  for (QString key : torrent_rules.keys()) {
    QJsonObject rule = torrent_rules.value(key).toObject();

    QMap<QString, QVariant> values;
    if (rule.value("rule_type").toString() == "advanced") {
      values.insert("regex", QRegExp(rule.value("regex").toString()));

      adv_rules.append(values);
    } else {
      values.insert("anime", rule.value("anime").toString());
      values.insert("subgroup", rule.value("subgroup").toString());
      values.insert("resolution", rule.value("resolution").toString());

      basic_rules.append(values);
    }
  }

  qDebug() << "Loaded" << adv_rules.count() << "advanced rules";
  qDebug() << "Loaded" << basic_rules.count() << "basic rules";
}

void MainWindow::checkForMatches() {
  reloadRules();

  for (int j = 0; j < ui->torrentTable->rowCount(); j++) {
    QString title = ui->torrentTable->item(j, 0)->text();
    QString sub   = ui->torrentTable->item(j, 2)->text();
    QString res   = ui->torrentTable->item(j, 3)->text();
    QString file  = ui->torrentTable->item(j, 4)->text();

    for (int i = 0; i < basic_rules.length(); i++) {
      if (QDate::currentDate().daysTo(
            QDate::fromString(basic_rules.at(i).value("expires", "2420-01-01")
                              .toString())) < 0) {
        basic_rules.removeAt(i);
        continue;
      }
      if (title == basic_rules.at(i).value("anime").toString() &&
          sub == basic_rules.at(i).value("subgroup").toString() &&
          res == basic_rules.at(i).value("resolution").toString()) {
        verifyAndDownload(j);
      }
    }

    for (int i = 0; i < adv_rules.length(); i++) {
      if (QDate::currentDate().daysTo
          (QDate::fromString(basic_rules.at(i).value("expires", "2420-01-01")
                             .toString())) < 0) {
        adv_rules.removeAt(i);
        continue;
      }

      if (adv_rules.at(i).value("regexp").toRegExp().exactMatch(file)) {
        verifyAndDownload(j);
      }
    }
  }
}

void MainWindow::verifyAndDownload(int row) {
  bool auto_download =
      settings->getValue(Settings::AutoDownload, true).toBool();
  bool auto_notify =
      settings->getValue(Settings::AutoNotify, false).toBool();

  QString title = ui->torrentTable->item(row, 4)->text();
  QString file = title + ".dl";
  QDir history_dir(QCoreApplication::applicationDirPath() + "/history/");
  history_dir.mkdir(".");
  QFile f(history_dir.absoluteFilePath(file));
  if (!f.exists()) {
    if (auto_notify) {
      trayIcon->showMessage("Shinjiru", tr("New matching torrents found."));
      return;
    }

    if (auto_download) {
      qDebug() << "Downloading" << title << "from torrent rule";
      download(row);
      download_rule++;
      rule_total++;

      ui->labelRulesLaunch->setText(QString::number(download_rule));
      ui->labelRulesTotal->setText(QString::number(rule_total));

      settings->setValue(Settings::RuleCount, rule_total);

      trayIcon->showMessage("Shinjiru", tr("%1 has started downloading.")
                                        .arg(title));
      f.open(QFile::WriteOnly);
      f.write("0");
    }
  }
}
