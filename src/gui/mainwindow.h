#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFuture>
#include <QFutureWatcher>
#include <QProgressBar>
#include <QTableWidget>
#include <QSignalMapper>
#include <QRegExp>
#include <QSystemTrayIcon>
#include <QJsonDocument>
#include <QJsonArray>
#include <QMenu>
#include <QApplication>
#include <QMessageBox>
#include <QElapsedTimer>

#include <QtAwesome.h>
#include <anilistapi.h>

#include "../settings.h"
#include "../api/user.h"
#include "../api/anime.h"
#include "airinganime.h"
#include "flowlayout.h"
#include "../anitomy/anitomywrapper.h"
#include "../lib/windowwatcher.h"
#include "progresstablewidgetitem.h"
#include "facetablewidgetitem.h"

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
  void changeEvent(QEvent *);

private:
  Ui::MainWindow *ui;
  QtAwesome *awesome;
  Settings *settings;
  WindowWatcher *window_watcher;
  AnitomyWrapper *anitomy;

  bool hasUser;

  FlowLayout *layout;

  QFuture<void> user_future;
  QFutureWatcher<void> user_future_watcher;

  QFuture<void> user_list_future;
  QFutureWatcher<void> user_list_future_watcher;

  QTimer *event_timer;
  QProgressBar *progress_bar;
  QElapsedTimer *uptime_timer;

  QTimer *watch_timer;
  QString currently_watching;
  QString cw_title;
  QString cw_episode;
  Anime *cw_anime;

  QString default_rule;
  QString title_style;

  int torrent_refresh_time;
  int torrent_interval;

  int download_count;
  int download_rule;
  int count_total;
  int rule_total;

  QList<QRegExp> adv_rules;
  QList<QMap<QString, QString>> basic_rules;

  QStringList airing;

  QSystemTrayIcon *trayIcon;
  QMenu *trayIconMenu;

  QAction *quitAction;
  QAction *minimizeAction;
  QAction *maximizeAction;
  QAction *restoreAction;

  int auto_update_delay;
  bool minimize_to_tray;
  bool close_to_tray;

private slots:
  void loadUser();
  void userLoaded();

  void loadUserList();
  void userListLoaded();

  void viewDashboard();
  void viewProfile();
  void viewAnimeList();

  void eventTick();

  void refreshTorrentListing();

  void toggleAnimeRecognition(bool);

  void showAnimeTab();
  void showSettingsTab();
  void showTorrentsTab();
  void showAiringTab();
  void showStatisticsTab();

  void showAbout();

  void showAnimePanel(int, int);

  QTableWidget *getListTable();

  AiringAnime *addAiring(Anime *);

  void watch(QString);
  void updateEpisode();

  void updateStatistics();

  void resetAPI();

  void filterTorrents(QString, bool);
  void filterTorrents(bool);
  void filterTorrents(QString);
  void torrentContextMenu(QPoint);
  void download(int);
  void createRule(int);
  void reloadRules();
  void checkForMatches();
  void verifyAndDownload(int);

  void settingsChanged();
  void applySettings();
  void applyEAR();
  void loadSettings();
  void defaultSettings();

  void initTray();
  void createActions();

  void moveUp();
  void moveDown();
};

#endif // MAINWINDOW_H
