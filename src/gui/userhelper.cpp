#include "ui_mainwindow.h"
#include "mainwindow.h"

#ifdef HAS_PREMIUM
  #include "../premium.h"
#endif

#include <QtConcurrent>

void MainWindow::loadUser() {
  if(!ui->actionRL->isEnabled()) return;

  progress_bar->setFormat(tr("Loading User"));
  progress_bar->setValue(15);
  ui->actionRL->setEnabled(false);
  user_future = QtConcurrent::run([&]() {
    if(this->hasUser) {
      User::sharedUser()->remake();
    } else
      User::sharedUser();
  });

  async_registry.append(user_future);

  user_future_watcher.setFuture(user_future);
}

void MainWindow::userLoaded() {
  progress_bar->setValue(20);
  progress_bar->setFormat(tr("User Loaded"));

  connect(User::sharedUser(), SIGNAL(reloadRequested()), this, SLOT(userListLoaded()));

  connect(User::sharedUser(), SIGNAL(new_image()), SLOT(repaint()));
  repaint();

  #ifdef HAS_PREMIUM
    if(!hasUser)
      Premium::sharedPremium()->loadPremium();
  #endif

  if(!hasUser) {
    loadUserList();
  } else {
    userListLoaded();
  }

  hasUser = true;
}
