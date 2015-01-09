#include "mainwindow.h"
#include "ui_mainwindow.h"

void MainWindow::loadSettings() {
  torrent_interval = settings->getValue(Settings::TorrentRefreshTime, 3600).toInt();
  ui->torrentRefreshIntervalLineEdit->setText(QString::number(torrent_interval));

  bool ear = settings->getValue(Settings::AnimeRecognitionEnabled, false).toBool();
  toggleAnimeRecognition(ear);

  this->default_rule = settings->getValue(Settings::DefaultRuleType, "Basic").toString();
  ui->defaultTorrentRuleModeComboBox->setCurrentText(default_rule);
}
