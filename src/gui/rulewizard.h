#ifndef RULEWIZARD_H
#define RULEWIZARD_H

#include <QDialog>

namespace Ui {
class RuleWizard;
}

class RuleWizard : public QDialog
{
  Q_OBJECT

public:
  explicit RuleWizard(QWidget *parent = 0, QString title = "", QString sub = "", QString res = "", QString file = "");
  ~RuleWizard();

private:
  Ui::RuleWizard *ui;

private slots:
  void groupToggle(bool);
  void group2Toggle(bool);
};

#endif // RULEWIZARD_H
