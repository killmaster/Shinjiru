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
};

#endif // RULEMANAGER_H
