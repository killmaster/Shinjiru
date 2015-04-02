#include "mainwindow.h"
#include "ui_mainwindow.h"

void MainWindow::initTray() {
  trayIconMenu = new QMenu(this);
  trayIconMenu->addAction(restoreAction);
  trayIconMenu->addSeparator();
  trayIconMenu->addAction(animeRecognitionAction);
  trayIconMenu->addAction(cancelUpdateAction);
  trayIconMenu->addSeparator();
  trayIconMenu->addAction(quitAction);

  trayIcon = new QSystemTrayIcon(this);
  trayIcon->setContextMenu(trayIconMenu);
  trayIcon->setIcon(windowIcon());
  trayIcon->setToolTip("Shinjiru " + qApp->applicationVersion());

  connect(trayIcon, &QSystemTrayIcon::activated, [&] (QSystemTrayIcon::ActivationReason reason) {
    if(reason == QSystemTrayIcon::DoubleClick) {
      if(showFunc.isEmpty()) showNormal();
      else QMetaObject::invokeMethod(this, showFunc.toLocal8Bit().data(),Qt::DirectConnection);
      qApp->setActiveWindow(this);
    }

    if(reason == QSystemTrayIcon::Context) {
      animeRecognitionAction->setChecked(ui->actionEAR->isChecked());
      cancelUpdateAction->setEnabled(watch_timer->isActive());
    }
  });
}


void MainWindow::createActions() {
  restoreAction = new QAction(tr("&Restore"), this);
  connect(restoreAction, SIGNAL(triggered()), this, SLOT(showNormal()));

  quitAction = new QAction(tr("&Quit"), this);
  connect(quitAction, &QAction::triggered, [&]() {
    this->elegantClose();
  });

  animeRecognitionAction = new QAction(tr("&Anime Recognition"), this);
  animeRecognitionAction->setCheckable(true);
  connect(animeRecognitionAction, &QAction::triggered, [&]() {
    this->toggleAnimeRecognition(animeRecognitionAction->isChecked());
  });

  cancelUpdateAction = new QAction(tr("&Cancel Update"), this);
  cancelUpdateAction->setEnabled(false);
  connect(cancelUpdateAction, &QAction::triggered, [&]() {
    watch_timer->stop();
  });
}
