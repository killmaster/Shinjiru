#include "api.h"
#include "../app.h"

#include <QDesktopServices>
#include <QInputDialog>

API::API(QObject *parent): QObject(parent) {
}

int API::verify(AniListAPI *api) {
  if(!api->hasAuthorizationCode()) {
    bool ok;
    QDesktopServices::openUrl(QUrl(appAuthURL));
    QString message = "Authorization code:                                                                                    ";
    QString text = QInputDialog::getText(static_cast<QWidget *>(this->parent()), tr("Authorization Code Request"), tr(message.toUtf8().data()), QLineEdit::Normal, "", &ok);

    if (ok && !text.isEmpty()) {
      api->setAuthorizationCode(text);
    } else {
      return AniListAPI::NO_AUTHORIZATION;
    }
  }

  return api->init();
}
