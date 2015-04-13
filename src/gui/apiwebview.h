/* Copyright 2015 Kazakuri */

#ifndef SRC_GUI_APIWEBVIEW_H_
#define SRC_GUI_APIWEBVIEW_H_

#include <QDialog>
#include <QUrl>
#include <QNetworkReply>

namespace Ui {
class APIWebView;
}

class APIWebView : public QDialog {
  Q_OBJECT

 public:
  explicit APIWebView(QWidget *parent = 0);
  ~APIWebView();

 private:
  Ui::APIWebView *ui;

 private slots:  // NOLINT
  void webURLChanged();
  void sslErrorHandler(QNetworkReply* qnr, const QList<QSslError> & errlist);
};

#endif  // SRC_GUI_APIWEBVIEW_H_
