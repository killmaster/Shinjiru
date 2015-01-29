#include "api.h"
#include "../app.h"

#include <QMutex>
#include <QApplication>
#include <QDesktopServices>
#include <QInputDialog>


#include "../lib/filedownloader.h"
#include "../gui/apiwebview.h"

API* API::m_Instance = 0;

API* API::sharedAPI() {
  static QMutex mutex;
  if(! m_Instance) {
    mutex.lock();

    if(!m_Instance) {
      m_Instance = new API;
    }

    mutex.unlock();
  }

  return m_Instance;
}

AniListAPI* API::sharedAniListAPI() {
  return m_API;
}

API::API() : QObject(0) {
  m_API = new AniListAPI(this, api_id, api_secret);
}

API::~API() {

}

int API::verify() {
  appUseWebView = false;

  if(!m_API->hasAuthorizationCode()) {
    if(appUseWebView) {
      APIWebView *wv = new APIWebView;
      wv->show();

      QEventLoop waiter;
      connect(wv, SIGNAL(accepted()), &waiter, SLOT(quit()));
      waiter.exec();

      delete wv;
    } else {
      bool ok;
      QDesktopServices::openUrl(QUrl(appAuthPINURL));

      QString message = "Authorization pin:                                                                                    ";
      QString text = QInputDialog::getText(static_cast<QWidget *>(this->parent()), tr("Authorization Pin Request"), tr(message.toUtf8().data()), QLineEdit::Normal, "", &ok);

      if (ok && !text.isEmpty()) {
        qDebug() << text;
        m_API->setAuthorizationPin(text);
      } else {
        return AniListAPI::NO_AUTHORIZATION;
      }
    }
  }

  if(appUseWebView) return m_API->init("code");

  return m_API->init("pin");
}
