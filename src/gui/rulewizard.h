/* Copyright 2015 Kazakuri */

#ifndef SRC_GUI_RULEWIZARD_H_
#define SRC_GUI_RULEWIZARD_H_

#include <QDialog>

namespace Ui {
class RuleWizard;
}

class RuleWizard : public QDialog {
  Q_OBJECT

 public:
  explicit RuleWizard(QWidget *parent, QString title,
                      QString sub, QString res, QString file,
                      QString default_rule);
  RuleWizard(QWidget *parent, QString file);
  ~RuleWizard();

  QString fileName() {
    return file_name;
  }

 private:
  Ui::RuleWizard *ui;
  bool edit_mode;
  QString file_name;

 private slots:  // NOLINT
  void groupToggle(bool on);
  void group2Toggle(bool on);
  void accept();
};

#endif  // SRC_GUI_RULEWIZARD_H_
