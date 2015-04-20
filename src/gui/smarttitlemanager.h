/* Copyright 2015 Kazakuri */

#ifndef SRC_GUI_SMARTTITLEMANAGER_H_
#define SRC_GUI_SMARTTITLEMANAGER_H_

#include <QDialog>

namespace Ui {
class SmartTitleManager;
}

class SmartTitleManager : public QDialog {
  Q_OBJECT

 public:
  explicit SmartTitleManager(QWidget *parent = 0);
  ~SmartTitleManager();

 private:
  Ui::SmartTitleManager *ui;

 private slots:  // NOLINT
  void updateName();
  void accept();
};

#endif  // SRC_GUI_SMARTTITLEMANAGER_H_
