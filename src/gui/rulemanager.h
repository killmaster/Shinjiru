/* Copyright 2015 Kazakuri */

#ifndef SRC_GUI_RULEMANAGER_H_
#define SRC_GUI_RULEMANAGER_H_

#include <QDialog>

namespace Ui {
class RuleManager;
}

class RuleManager : public QDialog {
  Q_OBJECT

 public:
  explicit RuleManager(QWidget *parent = 0, QString default_rule = "basic");
  ~RuleManager();

 private:
  Ui::RuleManager *ui;
  void reloadList();
};

#endif  // SRC_GUI_RULEMANAGER_H_
