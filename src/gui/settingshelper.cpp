/* Copyright 2015 Kazakuri */

#include "./mainwindow.h"
#include "./ui_mainwindow.h"

void MainWindow::loadSettings() {
  default_rule =
      settings->getValue(Settings::DefaultRuleType, "basic")
      .toString().toLower();
  bool ear =
      settings->getValue(Settings::AnimeRecognitionEnabled, false).toBool();
  QStringList list_order =
      settings->getValue(Settings::ListOrder, QStringList()).toStringList();
  count_total = settings->getValue(Settings::DownloadCount, 0).toInt();
  rule_total = settings->getValue(Settings::RuleCount, 0).toInt();
  QString release_stream =
      settings->getValue(Settings::ReleaseStream, "Release").toString();

  release_stream = release_stream == tr("Beta") ? "Nightly" : "Stable";

  ui->defaultTorrentRuleModeComboBox->setCurrentText(default_rule);
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

  QStringList list_order;

  for (int i = 0; i < ui->orderListWidget->count(); i++) {
    list_order << ui->orderListWidget->item(i)->text();
  }

  settings->setValue(Settings::TorrentRefreshTime, ti.toInt());
  settings->setValue(Settings::AnimeRecognitionEnabled,
                     ui->autoRecognitionCheckBox->isChecked());
  settings->setValue(Settings::DefaultRuleType,
                     ui->defaultTorrentRuleModeComboBox->currentText()
                     .toLower());
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
