/* Copyright 2015 Kazakuri */

#ifndef SRC_GUI_MAINWINDOW_H_
#define SRC_GUI_MAINWINDOW_H_

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
#include "./airinganime.h"
#include "./flowlayout.h"
#include "../anitomy/anitomywrapper.h"
#include "../lib/windowwatcher.h"
#include "./progresstablewidgetitem.h"
#include "./facetablewidgetitem.h"
#include "./overlay.h"
#include "./browseanime.h"
#include "../api/smarttitle.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

 protected:
  void closeEvent(QCloseEvent *e);
  void paintEvent(QPaintEvent *e);
  void resizeEvent(QResizeEvent *e);
  void changeEvent(QEvent *e);
  void mouseDoubleClickEvent(QMouseEvent *e);

 signals:
  void genres_loaded();

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
  int default_time;

  QAction *quitAction;
  QAction *restoreAction;
  QAction *animeRecognitionAction;
  QAction *cancelUpdateAction;

  int auto_update_delay;
  bool minimize_to_tray;
  bool close_to_tray;

  QString showFunc;

  QList<std::shared_ptr<QCheckBox>> genres;

  QList<QFuture<void>> async_registry;

 private slots:  // NOLINT
  void loadUser();
  void userLoaded();

  void loadUserList();
  void userListLoaded();

  void viewDashboard();
  void viewProfile();
  void viewAnimeList();

  void eventTick();

  void refreshTorrentListing();

  void toggleAnimeRecognition(bool ar);

  void showAnimeTab();
  void showSettingsTab();
  void showTorrentsTab();
  void showBrowseTab();
  void showAiringTab();
  void showStatisticsTab();

  void showAnimePanel(int row, int col);
  void showAnimePanel(int row, int col, QTableWidget *source);

  QTableWidget *getListTable(bool custom);

  AiringAnime *addAiring(Anime *anime);

  void watch(QString);
  void updateEpisode();

  void updateStatistics();

  void resetAPI();

  void reloadSmartTitles();

  void elegantClose();

  void filterTorrents(QString text, bool hide);
  void filterTorrents(bool hide);
  void filterTorrents(QString text);
  void torrentContextMenu(QPoint source);
  void download(int row);
  void createRule(int row);
  void reloadRules();
  void checkForMatches();
  void verifyAndDownload(int row);
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
  void addNoAnimePrompt();

  void filterList(int i);
  void filterList(QString text);
  void showSearch();

  void loadBrowserData();
  QUrl addPage(QUrl url, int page);
  QUrl addQuery(QUrl url, QString key, QString value);

  void exportListJSON();
};

#endif  // SRC_GUI_MAINWINDOW_H_
