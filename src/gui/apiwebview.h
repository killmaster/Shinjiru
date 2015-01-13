#ifndef APIWEBVIEW_H
#define APIWEBVIEW_H

#include <QDialog>
#include <QUrl>

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
};

#endif // APIWEBVIEW_H
