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

  QFuture<QJsonObject> userListJson;
  QFutureWatcher<QJsonObject> userListWatcher;

  QFutureWatcher<void> animeFutureData;

  QPixmap userImage;

  QStringList knownAnime;

  std::basic_string<wchar_t> toAnitomyFormat(QString);

  QList<Anime *> animeData;

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
  void loadAnimeData();
  void viewAnimeList();
  void resetProgress();
  void tick();
  void loadList();
  void updateProgess();
  void refreshList();
  void setUserImage();
  void filterTorrents(QString, bool);
  void filterTorrents(bool);
  void filterTorrents(QString);
  void torrentContextMenu(QPoint);
  void download(int);
  void createRule(int);
};

#endif // MAINWINDOW_H
