#include "mainwindow.h"
#include "ui_mainwindow.h"

void MainWindow::loadSettings() {
  torrent_interval = settings->getValue(Settings::TorrentRefreshTime, 900).toInt();
  default_rule = settings->getValue(Settings::DefaultRuleType, "Basic").toString();
  bool ear = settings->getValue(Settings::AnimeRecognitionEnabled, false).toBool();
  auto_update_delay = settings->getValue(Settings::AutoUpdateDelay, 120).toInt();
  minimize_to_tray = settings->getValue(Settings::MinimizeToTray, false).toBool();
  close_to_tray = settings->getValue(Settings::CloseToTray, true).toBool();
  bool sob = settings->getValue(Settings::StartOnBoot, false).toBool();

  if(sob) {
    /*
     * TODO: Check if it still starts up on boot
     */
  }

  ui->startOnBootCheckBox->setChecked(sob);
  ui->torrentRefreshIntervalLineEdit->setText(QString::number(torrent_interval));
  ui->defaultTorrentRuleModeComboBox->setCurrentText(default_rule);
  ui->autoUpdateDelayLineEdit->setText(QString::number(auto_update_delay));
  ui->minimizeToTrayCheckBox->setChecked(minimize_to_tray);
  ui->closeToTrayCheckBox->setChecked(close_to_tray);
  ui->startOnBootCheckBox->setChecked(sob);
  toggleAnimeRecognition(ear);
}

void MainWindow::settingsChanged() {
  ui->applyButton->setEnabled(true);
}

void MainWindow::applyEAR() {
  settings->setValue(Settings::AnimeRecognitionEnabled, ui->actionEAR->isChecked());
}

void MainWindow::applySettings() {
  QString ti = ui->torrentRefreshIntervalLineEdit->text();
  if(ti.toInt() <= 0) ti = "900";

  QString aud = ui->autoUpdateDelayLineEdit->text();
  if(aud.toInt() <= 0) aud = "120";

  bool sob = ui->startOnBootCheckBox->isChecked();

  if(sob) {
    /*
     * TODO: Make software start on boot
     */
  }

  minimize_to_tray = ui->minimizeToTrayCheckBox->isChecked();
  close_to_tray = ui->closeToTrayCheckBox->isChecked();

  settings->setValue(Settings::TorrentRefreshTime, ti.toInt());
  settings->setValue(Settings::AnimeRecognitionEnabled, ui->autoRecognitionCheckBox->isChecked());
  settings->setValue(Settings::DefaultRuleType, ui->defaultTorrentRuleModeComboBox->currentText());
  settings->setValue(Settings::AutoUpdateDelay, aud.toInt());
  settings->setValue(Settings::StartOnBoot, sob);
  settings->setValue(Settings::MinimizeToTray, minimize_to_tray);
  settings->setValue(Settings::CloseToTray, close_to_tray);

  ui->applyButton->setEnabled(false);
}

void MainWindow::defaultSettings() {
  /*
   * TODO: Show confirmation dialog
   */

  settings->setValue(Settings::TorrentRefreshTime,      900);
  settings->setValue(Settings::AnimeRecognitionEnabled, false);
  settings->setValue(Settings::DefaultRuleType,         "Basic");
  settings->setValue(Settings::AutoUpdateDelay,         120);
  settings->setValue(Settings::StartOnBoot,             false);
  settings->setValue(Settings::MinimizeToTray,          false);
  settings->setValue(Settings::CloseToTray,             true);

  QProcess::startDetached(QApplication::applicationFilePath());
  exit(0);
}
