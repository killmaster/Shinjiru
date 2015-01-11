#include "ui_mainwindow.h"
#include "mainwindow.h"

#include <QtConcurrent>

void MainWindow::loadUser() {
  progress_bar->setFormat("Loading User");
  progress_bar->setValue(15);
  user_future = QtConcurrent::run([&]() {
    user = new User();
    return user;
  });

  user_future_watcher.setFuture(user_future);
}

void MainWindow::userLoaded() {
  progress_bar->setValue(20);
  progress_bar->setFormat("User Loaded");

  connect(user, SIGNAL(new_image()), SLOT(repaint()));
  repaint();

  loadUserList();
}
