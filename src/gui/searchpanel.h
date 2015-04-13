/* Copyright 2015 Kazakuri */

#ifndef SRC_GUI_SEARCHPANEL_H_
#define SRC_GUI_SEARCHPANEL_H_

#include <QDialog>

namespace Ui {
class SearchPanel;
}

class SearchPanel : public QDialog {
  Q_OBJECT

 public:
  explicit SearchPanel(QWidget *parent = 0);
  ~SearchPanel();

  void setSearch(QString text);

 private:
  Ui::SearchPanel *ui;
};

#endif  // SRC_GUI_SEARCHPANEL_H_
