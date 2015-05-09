/* Copyright 2015 Kazakuri */

#include "./mainwindow.h"
#include "./ui_mainwindow.h"

void MainWindow::applyEAR() {
  settings->setValue(Settings::AnimeRecognitionEnabled,
                     ui->actionEAR->isChecked());
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
