#include "gui/mainwindow.h"
#include "app.h"
#include "fvupdater.h"
#include "lib/crashhandler/crash_handler.h"

#include <stdio.h>
#include <stdlib.h>

#include <QStandardPaths>
#include <QApplication>
#include <QCoreApplication>
#include <QFile>

void messageHandler(QtMsgType type, const QMessageLogContext &, const QString & str) {
  const char *msg = str.toStdString().c_str();

  QString parser(msg);

  if(parser.contains(api_id) || parser.contains(api_secret)) return;

  parser.replace(QString::fromWCharArray(L"\u2401"), "");
  parser.replace(QString::fromWCharArray(L"\u2406"), "");
  parser.replace(QString::fromWCharArray(L"\u2404"), "");
  parser.replace(QString::fromWCharArray(L"\u2405"), "");

  QFile logFile(qApp->applicationDirPath() + "/" + appName + ".log");
  logFile.open(QFile::WriteOnly | QFile::Append);

  switch (type) {
    case QtDebugMsg:
      logFile.write(QString("Debug: ").toUtf8() + parser.toUtf8() + QString("\n").toUtf8());
      break;
    case QtWarningMsg:
      logFile.write(QString("Warning: ").toUtf8() + parser.toUtf8() + QString("\n").toUtf8());
      break;
    case QtCriticalMsg:
      logFile.write(QString("Critical: ").toUtf8() + parser.toUtf8() + QString("\n").toUtf8());
      break;
    case QtFatalMsg:
      logFile.write(QString("Fatal: ").toUtf8() + parser.toUtf8() + QString("\n").toUtf8());
      abort();
  }
}

int main(int argc, char *argv[]) {
  qInstallMessageHandler(messageHandler);

  QApplication a(argc, argv);

  QFile logFile(qApp->applicationDirPath() + "/" + appName + ".log");
  if(logFile.exists()) logFile.remove();

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
