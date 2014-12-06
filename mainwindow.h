    #ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QTimer>
#include <QSignalMapper>
#include <QFuture>
#include <QFutureWatcher>
#include <QPaintEvent>
#include <QProgressBar>
#include <QMap>

#include "QtAwesome.h"
#include "anilistapi.h"
#include "filedownloader.h"
#include "anime.h"

namespace Ui {
  class MainWindow;
}

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

  int scoreType();

protected:
  void closeEvent(QCloseEvent *);
  void paintEvent(QPaintEvent *);

private:
  Ui::MainWindow *ui;

  QProgressBar *progressBar;

  void readSettings();
  void writeSettings();

  QtAwesome *awesome;
  QTimer *torrentRefreshTimer;
  QTimer *eventTimer;
  AniListAPI *api;

  QFuture<QJsonObject> userJson;
  QFutureWatcher<QJsonObject> userWatcher;
  FileDownloader *userImageCtrl;
  FileDownloader *animeImageCtrl;

  QFuture<QJsonObject> userListJson;
  QFutureWatcher<QJsonObject> userListWatcher;


  QFuture<QJsonObject> userAnimeData;
  QFutureWatcher<QJsonObject> animeDataWatcher;

  QFutureWatcher<void> animeFutureData;

  QPixmap userImage;

  QSet<QString> airingTitles;

  std::basic_string<wchar_t> toAnitomyFormat(QString);

  QMap<QString, Anime *> animeData;

  QStringList defaultListLabels;

  QString aniListDisplayName;
  int score_type;
  QString max_score;

  enum ErrorCode {
    NO_AUTHCODE,
    API_ERROR
  };

signals:
  void displayNameAvailable();

private slots:
  void showAnimeTab();
  void showSettingsTab();
  void showTorrentsTab();
  void refreshUser();
  void enableApply();
  void applySettings();
  void loadTorrents();
  void loadUser();
  void viewDashboard();
  void viewProfile();
  void viewAnimeList();
  void loadAnimeData(QString, QString);
  void resetProgress();
  void tick();
  void showAnimePanel(int, int);
  void loadList();
  void updateProgess();
  void refreshList();
  void setUserImage();
  void filterTorrents(QString, bool);
  void filterTorrents(bool);
  void filterTorrents(QString);
  void torrentContextMenu(QPoint);
  void download(int);
  void processAnimeData();
  void createRule(int);
};

#endif // MAINWINDOW_H
