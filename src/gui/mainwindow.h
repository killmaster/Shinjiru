#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFuture>
#include <QFutureWatcher>
#include <QProgressBar>
#include <QTableWidget>

#include <QtAwesome.h>
#include <anilistapi.h>

#include "../settings.h"
#include "../api/user.h"
#include "../api/anime.h"
#include "airinganime.h"
#include "flowlayout.h"

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
  void resizeEvent(QResizeEvent *);

private:
  Ui::MainWindow *ui;
  QtAwesome *awesome;
  AniListAPI *api;
  Settings *settings;
  User *user;

  FlowLayout *layout;

  QFuture<User *> user_future;
  QFutureWatcher<User *> user_future_watcher;

  QFuture<User *> user_list_future;
  QFutureWatcher<User *> user_list_future_watcher;

  QTimer *event_timer;
  QProgressBar *progress_bar;

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

  QTableWidget *getListTable();

  AiringAnime *addAiring(Anime *);
};

#endif // MAINWINDOW_H
