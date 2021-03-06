/* Copyright 2015 Kazakuri */

#include "./api.h"

#include <QMutex>
#include <QApplication>
#include <QDesktopServices>
#include <QInputDialog>

#include "../version.h"
#include "../app.h"
#include "../lib/filedownloader.h"
#include "../gui/apiwebview.h"

API* API::m_Instance = 0;

API* API::sharedAPI() {
  static QMutex mutex;
  if (!m_Instance) {
    mutex.lock();

    if (!m_Instance) {
      m_Instance = new API;
    }

    mutex.unlock();
  }

  return m_Instance;
}

void API::deleteInstance() {
  static QMutex mutex;
  mutex.lock();
  delete m_Instance;
  m_Instance = 0;
  mutex.unlock();
}

AniListAPI* API::sharedAniListAPI() {
  return m_API;
}

API::API() : QObject(0) {
  m_API = new AniListAPI(this, api_id, api_secret);
}

API::~API() {
  delete m_API;
}

int API::verifyAPI() {
  qDebug() << "Verifying API client with AniList...";

  if (!m_API->hasAuthorizationCode()) {
    qDebug() << "No authorization code found, prompting user for access";
    if (VER_USEWEBVIEW_BOOL == TRUE) {
      APIWebView *wv = new APIWebView;
      wv->show();

      QEventLoop waiter;
      connect(wv, SIGNAL(accepted()), &waiter, SLOT(quit()));
      waiter.exec();

      delete wv;
      wv = nullptr;
    } else {
      bool ok;
      QDesktopServices::openUrl(QUrl(VER_AUTHPINURL_STR));

      // NOLINTNEXTLINE
      QString message = "Authorization pin:                                                                                    ";
      QString text =
          QInputDialog::getText(0, tr("Authorization Pin Request"),
                                tr(message.toUtf8().data()), QLineEdit::Normal,
                                "", &ok);

      if (ok && !text.isEmpty()) {
        m_API->setAuthorizationPin(text);
      } else {
        return AniListAPI::NO_AUTHORIZATION;
      }
    }
  }

  if (VER_USEWEBVIEW_BOOL == TRUE) return m_API->init("code");

  return m_API->init("pin");
}
