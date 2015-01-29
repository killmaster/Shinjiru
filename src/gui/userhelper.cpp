#include "ui_mainwindow.h"
#include "mainwindow.h"

#include <QtConcurrent>

void MainWindow::loadUser() {
  progress_bar->setFormat("Loading User");
  progress_bar->setValue(15);
  user_future = QtConcurrent::run([&]() {
    if(this->hasUser) {
      User::sharedUser()->remake();
    }

    User::sharedUser();
  });

  user_future_watcher.setFuture(user_future);
}

void MainWindow::userLoaded() {
  hasUser = true;
  progress_bar->setValue(20);
  progress_bar->setFormat("User Loaded");

  connect(User::sharedUser(), SIGNAL(new_image()), SLOT(repaint()));
  repaint();

  if(needsRefresh) {
    loadUserList();
    needsRefresh = false;
  }
}
