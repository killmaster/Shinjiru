/* Copyright 2015 Kazakuri */

#include "./windowwatcher.h"

#include <QDebug>
#include <QProcess>
#include <QEventLoop>

#include <cstdlib>

WindowWatcher::WindowWatcher(QObject *parent) : QObject(parent) {
  timer = new QTimer(this);

  video = QRegExp("(.*(\\.mkv|\\.mp4|\\.avi)).*");
  exceptions = QRegExp("(\\.png|\\.gif|\\.jpg|\\.jpeg)");

  connect(timer, SIGNAL(timeout()), SLOT(timeOut()));

  timer->setInterval(5000);
}

#ifdef Q_OS_WIN
  BOOL CALLBACK EnumWindowsProc(HWND wnd, LPARAM lParam) {
    return reinterpret_cast<WindowWatcher*>(lParam)->parseWindow(wnd);
  }

  BOOL CALLBACK WindowWatcher::parseWindow(HWND hwnd) {
    int size = GetWindowTextLength(hwnd);
    if (size > 0 && IsWindowVisible(hwnd)) {
      wchar_t title[256];
      GetWindowText(hwnd, title, sizeof(title));
      windowList << QString::fromWCharArray(title);
    }

    return TRUE;
  }
#endif

void WindowWatcher::disable() {
  timer->stop();
}

void WindowWatcher::enable() {
  timer->start();
}

void WindowWatcher::timeOut() {
  windowList.clear();
  bool title = false;

  #ifdef Q_OS_WIN
    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(this));
  #endif

  #ifdef Q_OS_OSX
    QString script = "tell application \"System Events\" \
                       \rget name of every window of every process \
                     \rend tell";

    QProcess p;
    p.setProcessChannelMode(QProcess::MergedChannels);

    QEventLoop event;
    connect(&p, SIGNAL(finished(int)), &event, SLOT(quit()));
    p.start("osascript", QStringList() << "-e" << script);
    event.exec();

    QByteArray data = p.readAll();

    QString s(data);

    windowList = s.split(',');
  #endif

  for (int i = 0; i < windowList.length(); i++) {
    QString window = windowList.at(i);

    if (!isMediaPlayer(window)) continue;
    title = true;
    emit title_found(found_title);
    return;
  }

  if (!title) emit title_found("");
}

bool WindowWatcher::isMediaPlayer(QString window_title) {
  if (video.exactMatch(window_title)) {
    if (window_title.contains(exceptions)) return false;

    found_title = window_title;

    /* TODO
     * Fix for some basic title patterns, VLC etc
     */

    found_title = found_title.replace(" - VLC media player", "");

    return true;
  }
  return false;
}
