#include "gui/mainwindow.h"
#include "app.h"
#include "fvupdater.h"

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


  MainWindow w;
  w.show();

  return a.exec();
}
