#include "api.h"
#include "../app.h"

#include <QMutex>
#include <QApplication>
#include <QDesktopServices>
#include <QInputDialog>


#include "../lib/filedownloader.h"

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
  if(!m_API->hasAuthorizationCode()) {
    bool ok;
    QDesktopServices::openUrl(QUrl(appAuthURL));
    QString message = "Authorization code:                                                                                    ";
    QString text = QInputDialog::getText(static_cast<QWidget *>(this->parent()), tr("Authorization Code Request"), tr(message.toUtf8().data()), QLineEdit::Normal, "", &ok);

    if (ok && !text.isEmpty()) {
      m_API->setAuthorizationCode(text);
    } else {
      return AniListAPI::NO_AUTHORIZATION;
    }
  }

  return m_API->init();
}
