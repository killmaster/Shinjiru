#include "apiwebview.h"
#include "ui_apiwebview.h"

#include <QMessageBox>

#include "../api/api.h"
#include "../version.h"

APIWebView::APIWebView(QWidget *parent) : QDialog(parent), ui(new Ui::APIWebView) {
  ui->setupUi(this);

  ui->webView->load(QUrl(VER_AUTHURL_STR));
  ui->webView->show();

  connect(ui->webView, SIGNAL(urlChanged(QUrl)), SLOT(webURLChanged()));
  connect(ui->webView->page()->networkAccessManager(),
          SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError> & )),
          SLOT(sslErrorHandler(QNetworkReply*, const QList<QSslError> & ))
         );
}

APIWebView::~APIWebView()
{
  delete ui;
}

void APIWebView::webURLChanged() {
  QString s_url = ui->webView->url().toDisplayString();
  if(s_url.startsWith(VER_AUTHURL_STR) && s_url.contains("code")) {
    s_url = s_url.right(s_url.length() - s_url.indexOf("code") - 5);
    s_url = s_url.replace("&state=", "");

    API::sharedAPI()->sharedAniListAPI()->setAuthorizationCode(s_url);
    this->accept();
  }
}

void APIWebView::sslErrorHandler(QNetworkReply* qnr, const QList<QSslError> & errlist) {
  for(QSslError e: errlist) {
    qWarning() << e.errorString();
  }

  qnr->ignoreSslErrors();
}
