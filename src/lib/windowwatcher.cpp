#include "windowwatcher.h"

WindowWatcher::WindowWatcher(QObject *parent) : QObject(parent) {
  timer = new QTimer(this);

  connect(timer, SIGNAL(timeout()), SLOT(timeOut()));

  timer->setInterval(5000);
}

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

void WindowWatcher::disable() {
  timer->stop();
}

void WindowWatcher::enable() {
  timer->start();
}

void WindowWatcher::timeOut() {
  windowList.clear();
  bool title = false;

  EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(this));

  for(int i = 0; i < windowList.length(); i++) {
    QString window = windowList.at(i);

    if(!isMediaPlayer(window)) continue;
    title = true;
    emit title_found(found_title);
  }

  if(!title) emit title_found("");
}

bool WindowWatcher::isMediaPlayer(QString window_title) {
  QRegExp video("(.*(\\.mkv|\\.mp4|\\.avi)).*");
  if(video.exactMatch(window_title)) {
    found_title = window_title;

    /* TODO
     * Fix for some basic title patterns, VLC etc
     */

    return true;
  }
  return false;
}
