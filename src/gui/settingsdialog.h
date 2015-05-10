#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QtAwesome.h>
#include <QJsonObject>

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
  Q_OBJECT

public:
  explicit SettingsDialog(QWidget *parent = 0);
  ~SettingsDialog();

  void showSmartTitles();

private:
  Ui::SettingsDialog *ui;

  void loadSettings();
  void defaultSettings();
  void applySettings();

  QtAwesome *awesome;

  QList<QMap<QString, QVariant>> basic_rules;
  QList<QMap<QString, QVariant>> adv_rules;

  QJsonObject torrent_rules;

private slots:
  void accept();

  void moveUp();
  void moveDown();

  void loadTorrentRules();
  void saveTorrentRules();

  void loadSmartTitles();
  void saveSmartTitles();

  void updateSmartTitleName();
};

#endif // SETTINGSDIALOG_H
