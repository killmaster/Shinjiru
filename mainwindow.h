    #ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QTimer>
#include <QSignalMapper>

#include "QtAwesome.h"

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

private:
  Ui::MainWindow *ui;

  void readSettings();
  void writeSettings();

  QtAwesome *awesome;
  QTimer *torrentRefreshTimer;
  QTimer *eventTimer;

  std::basic_string<wchar_t> toAnitomyFormat(QString);

signals:
  void logged_in();

private slots:
  void showAnimeTab();
  void showSettingsTab();
  void showTorrentsTab();
  void refreshAll();
  void enableApply();
  void applySettings();
  void loadTorrents();
  void tick();
  void filterTorrents(QString);
  void torrentContextMenu(QPoint);
  void download(int);
  void createRule(int);
};

#endif // MAINWINDOW_H
