#include "mainwindow.h"

void MainWindow::initTray() {
  trayIconMenu = new QMenu(this);
  trayIconMenu->addAction(restoreAction);
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
  });
}


void MainWindow::createActions() {
  restoreAction = new QAction(tr("&Restore"), this);
  connect(restoreAction, SIGNAL(triggered()), this, SLOT(showNormal()));

  quitAction = new QAction(tr("&Quit"), this);
  connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
}
