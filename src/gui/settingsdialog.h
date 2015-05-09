#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QtAwesome.h>

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
  Q_OBJECT

public:
  explicit SettingsDialog(QWidget *parent = 0);
  ~SettingsDialog();

private:
  Ui::SettingsDialog *ui;
  void loadSettings();
  void defaultSettings();
  void applySettings();

  QtAwesome *awesome;

private slots:
  void accept();
  void moveUp();
  void moveDown();
};

#endif // SETTINGSDIALOG_H
