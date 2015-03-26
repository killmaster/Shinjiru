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
#include "overlay.h"
#include "seasonanime.h"

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
  void mouseDoubleClickEvent(QMouseEvent *);

private:
  Ui::MainWindow *ui;
  QtAwesome *awesome;
  Settings *settings;
  WindowWatcher *window_watcher;
  AnitomyWrapper *anitomy;
  Overlay *over;

  bool hasUser;

  FlowLayout *layout;
  FlowLayout *layout2;

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

  QList<QMap<QString, QVariant>> adv_rules;
  QList<QMap<QString, QVariant>> basic_rules;

  QList<AiringAnime *> airing_anime;

  QStringList airing;

  QSystemTrayIcon *trayIcon;
  QMenu *trayIconMenu;

  int user_refresh_time;

  QAction *quitAction;
  QAction *restoreAction;
  QAction *animeRecognitionAction;
  QAction *cancelUpdateAction;

  int auto_update_delay;
  bool minimize_to_tray;
  bool close_to_tray;

  QString showFunc;

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
  void showSeasonsTab();
  void showAiringTab();
  void showStatisticsTab();

  void showAnimePanel(int, int);
  void showAnimePanel(int, int, QTableWidget *);

  QTableWidget *getListTable(bool);

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
  void showRuleManager();

  void settingsChanged();
  void applySettings();
  void applyEAR();
  void loadSettings();
  void defaultSettings();

  void initTray();
  void createActions();

  void updateStatus(int row, QTableWidget *table, QString status);

  void moveUp();
  void moveDown();

  void addSearchPrompt();

  void filterList(int);
  void filterList(QString);
  void showSearch();

  void loadSelectedSeason();

  void exportListJSON();
};

#endif // MAINWINDOW_H
