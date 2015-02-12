#include "gui/mainwindow.h"
#include "version.h"
#include "app.h"
#include "fvupdater.h"

#ifndef Q_OS_OSX
  #include "lib/crashhandler/crash_handler.h"
#endif

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

  QFile logFile(qApp->applicationDirPath() + "/" + VER_PRODUCTNAME_STR + ".log");
  logFile.open(QFile::WriteOnly | QFile::Append);

  logFile.write(QDateTime::currentDateTimeUtc().toString("[yyyy-MM-dd HH:mm:ss.zzz] ").toUtf8());

  switch (type) {
    case QtDebugMsg:
      logFile.write(QString("Debug: ").toUtf8());
      break;
    case QtWarningMsg:
      logFile.write(QString("Warning: ").toUtf8());
      break;
    case QtCriticalMsg:
      logFile.write(QString("Critical: ").toUtf8());
      break;
    case QtFatalMsg:
      logFile.write(QString("Fatal: ").toUtf8());
      abort();
  }

  logFile.write(parser.toUtf8() + QString("\n").toUtf8());
}

int main(int argc, char *argv[]) {
  #ifndef Q_OS_OSX
    qInstallMessageHandler(messageHandler);
  #endif

  #ifdef Q_OS_OSX
    QSslConfiguration config = QSslConfiguration::defaultConfiguration();
    config.setProtocol(QSsl::AnyProtocol);
    QSslConfiguration::setDefaultConfiguration(config);
  #endif

  QApplication a(argc, argv);

  QFile logFile(qApp->applicationDirPath() + "/" + VER_PRODUCTNAME_STR + ".log");
  if(logFile.exists()) logFile.remove();

  Settings *s = new Settings(0);
  QString release_stream = s->getValue(Settings::ReleaseStream, "Release").toString();

  QCoreApplication::setApplicationName(VER_PRODUCTNAME_STR);
  QCoreApplication::setApplicationVersion(VER_PRODUCTVERSION_STR);
  QCoreApplication::setOrganizationName(VER_COMPANYNAME_STR);
  QCoreApplication::setOrganizationDomain(VER_COMPANYDOMAIN_STR);

  if(release_stream == "Release")
    FvUpdater::sharedUpdater()->SetFeedURL(VER_UPDATEFEED_STR);
  else if(release_stream == "Beta")
    FvUpdater::sharedUpdater()->SetFeedURL(VER_UPDATEFEED_STR_BETA);
  else
    FvUpdater::sharedUpdater()->SetFeedURL(VER_UPDATEFEED_STR);

  FvUpdater::sharedUpdater()->CheckForUpdatesSilent();

  #ifndef Q_OS_OSX
    Breakpad::CrashHandler::instance()->Init(qApp->applicationDirPath());
  #endif

  MainWindow w;
  w.show();

  return a.exec();
}
