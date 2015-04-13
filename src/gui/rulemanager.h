/* Copyright 2015 Kazakuri */

#ifndef RULEMANAGER_H
#define RULEMANAGER_H

#include <QDialog>

namespace Ui {
class RuleManager;
}

class RuleManager : public QDialog
{
  Q_OBJECT

public:
  explicit RuleManager(QWidget *parent = 0, QString default_rule = "basic");
  ~RuleManager();

private:
  Ui::RuleManager *ui;
  void reloadList();
};

#endif // RULEMANAGER_H
