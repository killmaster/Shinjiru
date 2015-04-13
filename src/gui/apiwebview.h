/* Copyright 2015 Kazakuri */

#ifndef APIWEBVIEW_H
#define APIWEBVIEW_H

#include <QDialog>
#include <QUrl>
#include <QNetworkReply>

namespace Ui {
class APIWebView;
}

class APIWebView : public QDialog
{
  Q_OBJECT

public:
  explicit APIWebView(QWidget *parent = 0);
  ~APIWebView();

private:
  Ui::APIWebView *ui;

private slots:
  void webURLChanged();
  void sslErrorHandler(QNetworkReply* qnr, const QList<QSslError> & errlist);
};

#endif // APIWEBVIEW_H
