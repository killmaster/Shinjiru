/* Copyright 2015 Kazakuri */

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
  explicit RuleWizard(QWidget *parent, QString title, QString sub, QString res, QString file, QString default_rule);
  RuleWizard(QWidget *parent, QString file);
  ~RuleWizard();

  QString fileName() {
    return file_name;
  }

private:
  Ui::RuleWizard *ui;
  bool edit_mode;
  QString file_name;

private slots:
  void groupToggle(bool);
  void group2Toggle(bool);
  void accept();
};

#endif // RULEWIZARD_H
