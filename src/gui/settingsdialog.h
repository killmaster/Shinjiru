/* Copyright 2015 Kazakuri */

#ifndef SRC_GUI_SETTINGSDIALOG_H_
#define SRC_GUI_SETTINGSDIALOG_H_

#include <QDialog>
#include <QtAwesome.h>
#include <QJsonObject>

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog {
  Q_OBJECT

 public:
  explicit SettingsDialog(QWidget *parent = 0);
  ~SettingsDialog();

  void showSmartTitles();
  void showTorrentRules(QString title, QString sub, QString res, QString file);

 private:
  Ui::SettingsDialog *ui;

  void loadSettings();
  void applySettings();
  void setAdvancedSetting(QString key, QString value);
  QString getAdvancedSetting(QString key);

  QtAwesome *awesome;

  QList<QMap<QString, QVariant>> basic_rules;
  QList<QMap<QString, QVariant>> adv_rules;

  QJsonObject torrent_rules;
  QString current_rule;

 private slots:  // NOLINT
  void accept();

  void defaultSettings();

  void moveUp();
  void moveDown();

  void loadTorrentRules();
  void saveTorrentRules();

  void toggleBasic(bool en);
  void toggleAdvanced(bool en);

  void loadSmartTitles();
  void saveSmartTitles();

  void updateSmartTitleName();

  void resetAPI();
};

#endif  // SRC_GUI_SETTINGSDIALOG_H_
