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

  QPixmap *pix = new QPixmap(width(), height());
  pix->fill(Qt::transparent);

  QPainter p(pix);
  p.setPen(QColor(30,30,30));
  p.setBrush(QColor(255,255,255));

  int notification_count = User::sharedUser()->notificationCount();
  if(notification_count > 0) p.setBrush(QColor(255,120,120));
  p.drawEllipse(width() - 25.0f,60.0f, 20.0f, 20.0f);
  QString notif = notification_count >= 9 ? "9+" : QString::number(notification_count);
  p.drawText(width() - 25.0f, 59.0f, 20.0f, 20.0f, Qt::AlignCenter, notif);

  over->addDrawing("notifications", pix);

  connect(User::sharedUser(), SIGNAL(new_image()), SLOT(repaint()));
  repaint();

  if(needsRefresh) {
    loadUserList();
    needsRefresh = false;
  }
}
