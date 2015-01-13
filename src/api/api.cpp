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
  if(!m_API->hasAuthorizationCode()) {
    APIWebView *wv = new APIWebView;
    wv->show();

    QEventLoop waiter;
    connect(wv, SIGNAL(accepted()), &waiter, SLOT(quit()));
    waiter.exec();

    delete wv;
  }

  return m_API->init();
}
