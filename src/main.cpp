#include "gui/mainwindow.h"
#include "app.h"
#include "fvupdater.h"
#include "lib/crashhandler/crash_handler.h"

#include <QStandardPaths>
#include <QApplication>
#include <QCoreApplication>

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);

  QCoreApplication::setApplicationName(appName);
  QCoreApplication::setApplicationVersion(appVersion);
  QCoreApplication::setOrganizationName(appAuthor);
  QCoreApplication::setOrganizationDomain(appDomain);

  FvUpdater::sharedUpdater()->SetFeedURL(appUpdateFeed);
  FvUpdater::sharedUpdater()->CheckForUpdatesSilent();

  Breakpad::CrashHandler::instance()->Init(qApp->applicationDirPath());

  MainWindow w;
  w.show();

  return a.exec();
}
