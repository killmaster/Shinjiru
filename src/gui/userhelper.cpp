/* Copyright 2015 Kazakuri */

#include <QtConcurrent>

#include "./ui_mainwindow.h"
#include "./mainwindow.h"

void MainWindow::loadUser() {
  if (!ui->actionRL->isEnabled()) return;

  progress_bar->setFormat(tr("Loading User"));
  progress_bar->setValue(15);
  ui->actionRL->setEnabled(false);
  user_future = QtConcurrent::run([&]() {  // NOLINT
    if (this->hasUser) {
      User::sharedUser()->remake();
    } else {
      User::sharedUser();
    }
  });

  for (QFuture<void> f : async_registry) {
    if (f.isFinished()) {
      async_registry.removeAll(f);
    }
  }

  async_registry.append(user_future);

  user_future_watcher.setFuture(user_future);
}

void MainWindow::userLoaded() {
  progress_bar->setValue(20);
  progress_bar->setFormat(tr("User Loaded"));

  connect(User::sharedUser(), SIGNAL(reloadRequested()),
          this, SLOT(userListLoaded()));

  connect(User::sharedUser(), SIGNAL(new_image()), SLOT(repaint()));
  repaint();

  if (!hasUser) {
    loadUserList();
  } else {
    userListLoaded();
  }

  hasUser = true;
}
