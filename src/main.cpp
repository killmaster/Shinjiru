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

#ifdef QT_DEBUG
#include <QMessageBox>
#include <QThread>
#include <iostream>

void noisyFailureMsgHandler(QtMsgType type, const QMessageLogContext &, const QString & str) {
  const char * msgAsCstring = str.toStdString().c_str();

  QString msg (msgAsCstring);
  std::cerr << msgAsCstring << std::endl;
  std::cerr.flush();

  if ((type == QtDebugMsg) && msg.contains("::connect")) {
    type = QtWarningMsg;
  }

  if ((type == QtDebugMsg)
      && msg.contains("QPainter::begin")
      && msg.contains("Paint device returned engine")) {
    type = QtWarningMsg;
  }

  if ((type == QtWarningMsg)
       && QString(msg).contains("QClipboard::event")
       && QString(msg).contains("Cowardly refusing")) {
    type = QtDebugMsg;
  }

  QCoreApplication * instance = QCoreApplication::instance();
  const bool isGuiThread = instance && (QThread::currentThread() == instance->thread());

  if (isGuiThread) {
    QMessageBox messageBox;
    switch (type) {
      case QtDebugMsg:
        return;
      case QtWarningMsg:
        messageBox.setIcon(QMessageBox::Warning);
        messageBox.setInformativeText(msg);
        messageBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        break;
      case QtCriticalMsg:
        messageBox.setIcon(QMessageBox::Critical);
        messageBox.setInformativeText(msg);
        messageBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        break;
      case QtFatalMsg:
        messageBox.setIcon(QMessageBox::Critical);
        messageBox.setInformativeText(msg);
        messageBox.setStandardButtons(QMessageBox::Cancel);
        break;
    }

    int ret = messageBox.exec();
    if (ret == QMessageBox::Cancel)
      abort();
  } else {
    if (type != QtDebugMsg)
      abort();
  }
}
#endif

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
  #ifdef Q_OS_OSX
    QSslConfiguration config = QSslConfiguration::defaultConfiguration();
    config.setProtocol(QSsl::AnyProtocol);
    QSslConfiguration::setDefaultConfiguration(config);
  #endif

  if(api_id.isEmpty() || api_secret.isEmpty()) {
    qFatal("No API key values!");
  }

  QApplication a(argc, argv);

  #ifndef Q_OS_OSX
    qInstallMessageHandler(messageHandler);
  #endif

  #ifdef QT_DEBUG
    qInstallMessageHandler(noisyFailureMsgHandler);
  #endif

  QFile logFile(qApp->applicationDirPath() + "/" + VER_PRODUCTNAME_STR + ".log");
  if(logFile.exists()) logFile.remove();

  QCoreApplication::setApplicationName(VER_PRODUCTNAME_STR);
  QCoreApplication::setApplicationVersion(VER_PRODUCTVERSION_STR);
  QCoreApplication::setOrganizationName(VER_COMPANYNAME_STR);
  QCoreApplication::setOrganizationDomain(VER_COMPANYDOMAIN_STR);

  Settings *s = new Settings;
  QString release_stream = s->getValue(Settings::ReleaseStream, "Release").toString();

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
