/* Copyright 2015 Kazakuri */

#include "./mainwindow.h"
#include "./ui_mainwindow.h"

void MainWindow::applyEAR() {
  settings->setValue(Settings::AnimeRecognitionEnabled,
                     ui->actionEAR->isChecked());
}
