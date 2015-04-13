/* Copyright 2015 Kazakuri */

#ifndef SRC_GUI_ABOUT_H_
#define SRC_GUI_ABOUT_H_

#include <QDialog>
#include <QPixmap>

namespace Ui {
class About;
}

class About : public QDialog {
  Q_OBJECT

 public:
  explicit About(QWidget *parent = 0);
  ~About();

 protected:
  void paintEvent(QPaintEvent *e);

 private:
  Ui::About *ui;
  QPixmap mascot;
};

#endif  // SRC_GUI_ABOUT_H_
