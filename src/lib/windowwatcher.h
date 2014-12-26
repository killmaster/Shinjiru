#ifndef WINDOWWATCHER_H
#define WINDOWWATCHER_H

#include <QObject>
#include <QStringList>
#include <QTimer>

#include <qt_windows.h>

class WindowWatcher : public QObject
{
  Q_OBJECT
public:
  explicit WindowWatcher(QObject *parent = 0);
  BOOL CALLBACK parseWindow(HWND hwnd);

signals:
  void title_found(QString);

public slots:
  void timeOut();

private:
  QTimer *timer;
  QStringList windowList;
  bool isMediaPlayer(QString window_title);
  QString found_title;
};

#endif // WINDOWWATCHER_H
