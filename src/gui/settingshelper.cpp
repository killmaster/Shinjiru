/* Copyright 2015 Kazakuri */

#include "./mainwindow.h"
#include "./ui_mainwindow.h"

void MainWindow::loadSettings() {
  torrent_interval =
      settings->getValue(Settings::TorrentRefreshTime, 900).toInt();
  default_rule =
      settings->getValue(Settings::DefaultRuleType, "basic")
      .toString().toLower();
  bool ear =
      settings->getValue(Settings::AnimeRecognitionEnabled, false).toBool();
  auto_update_delay =
      settings->getValue(Settings::AutoUpdateDelay, 120).toInt();
  minimize_to_tray =
      settings->getValue(Settings::MinimizeToTray, false).toBool();
  close_to_tray = settings->getValue(Settings::CloseToTray, true).toBool();
  bool sob = settings->getValue(Settings::StartOnBoot, false).toBool();
  QStringList list_order =
      settings->getValue(Settings::ListOrder, QStringList()).toStringList();
  count_total = settings->getValue(Settings::DownloadCount, 0).toInt();
  rule_total = settings->getValue(Settings::RuleCount, 0).toInt();
  QString release_stream =
      settings->getValue(Settings::ReleaseStream, "Release").toString();

  release_stream = release_stream == tr("Beta") ? "Nightly" : "Stable";

  if (sob) {
    #ifdef Q_OS_WIN
      QSettings reg("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\" +
                    "CurrentVersion\\Run", QSettings::NativeFormat);
      QString path = reg.value("Shinjiru", QString("")).toString();
      if (path.isEmpty()) sob = false;
    #endif
  }

  ui->startOnBootCheckBox->setChecked(sob);
  ui->torrentRefreshIntervalLineEdit->setText(
        QString::number(torrent_interval));
  ui->defaultTorrentRuleModeComboBox->setCurrentText(default_rule);
  ui->autoUpdateDelayLineEdit->setText(QString::number(auto_update_delay));
  ui->minimizeToTrayCheckBox->setChecked(minimize_to_tray);
  ui->closeToTrayCheckBox->setChecked(close_to_tray);
  ui->startOnBootCheckBox->setChecked(sob);
  ui->labelDownloadedTotal->setText(QString::number(count_total));
  ui->labelRulesTotal->setText(QString::number(rule_total));
  ui->updateStreamComboBox->setCurrentText(release_stream);
  toggleAnimeRecognition(ear);

  for (QString key : list_order) {
    ui->orderListWidget->addItem(key);
  }
}

void MainWindow::settingsChanged() {
  ui->applyButton->setEnabled(true);
}

void MainWindow::applyEAR() {
  settings->setValue(Settings::AnimeRecognitionEnabled,
                     ui->actionEAR->isChecked());
}

void MainWindow::applySettings() {
  QString ti = ui->torrentRefreshIntervalLineEdit->text();
  if (ti.toInt() <= 0) ti = "900";

  QString aud = ui->autoUpdateDelayLineEdit->text();
  if (aud.toInt() <= 0) aud = "120";

  bool sob = ui->startOnBootCheckBox->isChecked();

  QStringList list_order;

  for (int i = 0; i < ui->orderListWidget->count(); i++) {
    list_order << ui->orderListWidget->item(i)->text();
  }

  if (sob) {
    #ifdef Q_OS_WIN
      QSettings reg("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\" +
                    "CurrentVersion\\Run", QSettings::NativeFormat);
      reg.setValue("Shinjiru", "\"" +
                   qApp->applicationFilePath().replace("/", "\\") + "\"");
    #endif
  } else {
    #ifdef Q_OS_WIN
      QSettings reg("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\" +
                    "CurrentVersion\\Run", QSettings::NativeFormat);
      reg.remove("Shinjiru");
    #endif
  }

  minimize_to_tray = ui->minimizeToTrayCheckBox->isChecked();
  close_to_tray = ui->closeToTrayCheckBox->isChecked();

  settings->setValue(Settings::TorrentRefreshTime, ti.toInt());
  settings->setValue(Settings::AnimeRecognitionEnabled,
                     ui->autoRecognitionCheckBox->isChecked());
  settings->setValue(Settings::DefaultRuleType,
                     ui->defaultTorrentRuleModeComboBox->currentText()
                     .toLower());
  settings->setValue(Settings::AutoUpdateDelay, aud.toInt());
  settings->setValue(Settings::StartOnBoot, sob);
  settings->setValue(Settings::MinimizeToTray, minimize_to_tray);
  settings->setValue(Settings::CloseToTray, close_to_tray);
  settings->setValue(Settings::ListOrder, list_order);
  settings->setValue(Settings::ReleaseStream,
                     ui->updateStreamComboBox->currentText() == "Nightly" ?
                       "Beta" : "Release");

  ui->applyButton->setEnabled(false);
}

void MainWindow::defaultSettings() {
  if (QMessageBox::Yes ==
      QMessageBox::question(this, "Shinjiru",
      tr("Are you sure you want to reset the settings to their default value?"),
      QMessageBox::Yes|QMessageBox::No)) {
    settings->setValue(Settings::TorrentRefreshTime,      900);
    settings->setValue(Settings::AnimeRecognitionEnabled, false);
    settings->setValue(Settings::DefaultRuleType,         "basic");
    settings->setValue(Settings::AutoUpdateDelay,         120);
    settings->setValue(Settings::StartOnBoot,             false);
    settings->setValue(Settings::MinimizeToTray,          false);
    settings->setValue(Settings::CloseToTray,             true);
    settings->setValue(Settings::ListOrder,               QStringList());

    QProcess::startDetached(QApplication::applicationFilePath());
    exit(0);
  }
}

void MainWindow::resetAPI() {
  API::sharedAPI()->sharedAniListAPI()->setAuthorizationCode("");
  API::sharedAPI()->sharedAniListAPI()->setAuthorizationPin("");

  settings->setValue(Settings::AniListAccess, "");
  settings->setValue(Settings::AniListExpires, QDateTime::currentDateTimeUtc());
  settings->setValue(Settings::AniListRefresh, "");

  QProcess::startDetached(QApplication::applicationFilePath());
  exit(0);
}

void MainWindow::moveUp() {
  if (ui->orderListWidget->selectedItems().count() == 1) {
    int row =
       ui->orderListWidget->row(ui->orderListWidget->selectedItems().at(0));
    if (row != 0) {
      ui->orderListWidget->insertItem(
           row - 1, ui->orderListWidget->takeItem(row)->text());
      ui->orderListWidget->setCurrentRow(row - 1);
      this->settingsChanged();
    }
  }
}

void MainWindow::moveDown() {
  if (ui->orderListWidget->selectedItems().count() == 1) {
    int row =
        ui->orderListWidget->row(ui->orderListWidget->selectedItems().at(0));
    if (row != ui->orderListWidget->count()) {
      ui->orderListWidget->insertItem(
            row + 1, ui->orderListWidget->takeItem(row)->text());
      ui->orderListWidget->setCurrentRow(row + 1);
      this->settingsChanged();
    }
  }
}
