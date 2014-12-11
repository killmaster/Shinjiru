#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QTimer>
#include <QFuture>
#include <QFutureWatcher>
#include <QPaintEvent>
#include <QProgressBar>

#include <QtAwesome.h>
#include <anilistapi.h>

#include "../api/user.h"
#include "../lib/filedownloader.h"
#include "../api/anime.h"
#include "../settings.h"

namespace Ui {
  class MainWindow;
}

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

protected:
  void closeEvent(QCloseEvent *);
  void paintEvent(QPaintEvent *);

private:
  Ui::MainWindow *ui;
  QtAwesome *awesome;
  AniListAPI *api;
  Settings *settings;
  User *user;

  QFuture<User *> user_future;
  QFutureWatcher<User *> user_future_watcher;

  QFuture<User *> user_list_future;
  QFutureWatcher<User *> user_list_future_watcher;

  QTimer *event_timer;
  QProgressBar *progress_bar;

signals:

private slots:
  void loadUser();
  void userLoaded();

  void loadUserList();
  void userListLoaded();

  void viewDashboard();
  void viewProfile();
  void viewAnimeList();

  void showAnimeTab();
  void showSettingsTab();
  void showTorrentsTab();
  void showAiringTab();
  void showStatisticsTab();

  void showAnimePanel(int, int);
};

#endif // MAINWINDOW_H
