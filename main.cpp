#include "mainwindow.h"
#include "app.h"

#include <QApplication>
#include <QCoreApplication>

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);

  QCoreApplication::setApplicationName(appName);
  QCoreApplication::setApplicationVersion(appVersion);
  QCoreApplication::setOrganizationName(appAuthor);

  MainWindow w;
  w.show();

  return a.exec();
}
