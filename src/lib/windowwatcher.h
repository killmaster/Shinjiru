/* Copyright 2015 Kazakuri */

#ifndef SRC_LIB_WINDOWWATCHER_H_
#define SRC_LIB_WINDOWWATCHER_H_

#include <QObject>
#include <QStringList>
#include <QTimer>

#ifdef Q_OS_WIN
  #include <qt_windows.h>
#endif

class WindowWatcher : public QObject {
  Q_OBJECT

 public:
  explicit WindowWatcher(QObject *parent = 0);
  #ifdef Q_OS_WIN
    BOOL CALLBACK parseWindow(HWND hwnd);
  #endif

 signals:
  void title_found(QString);

 public slots:  // NOLINT
  void timeOut();
  void enable();
  void disable();

 private:
  QTimer *timer;
  QStringList windowList;
  bool isMediaPlayer(QString window_title);
  QString found_title;

  QRegExp video;
  QRegExp exceptions;
};

#endif  // SRC_LIB_WINDOWWATCHER_H_
