#include "./settingsdialog.h"
#include "./ui_settingsdialog.h"

#include "../settings.h"
#include "../lib/skinmanager.h"

SettingsDialog::SettingsDialog(QWidget *parent) :
  QDialog(parent), ui(new Ui::SettingsDialog) {
  ui->setupUi(this);

  ui->settingsTypeList->item(0)->setIcon(QIcon(QPixmap(
            SkinManager::sharedManager()->get(SkinManager::ApplicationIcon))));
  ui->settingsTypeList->item(1)->setIcon(QIcon(QPixmap(
            SkinManager::sharedManager()->get(SkinManager::AnimeListIcon))));
  ui->settingsTypeList->item(2)->setIcon(QIcon(QPixmap(
            SkinManager::sharedManager()->get(SkinManager::RecognitionIcon))));
  ui->settingsTypeList->item(3)->setIcon(QIcon(QPixmap(
            SkinManager::sharedManager()->get(SkinManager::TorrentsIcon))));
  ui->settingsTypeList->item(4)->setIcon(QIcon(QPixmap(
            SkinManager::sharedManager()->get(SkinManager::AdvancedIcon))));

  connect(ui->settingsTypeList, &QListWidget::currentRowChanged,
    [&](int row) {  //NOLINT
    ui->settingsTypeTabs->setCurrentIndex(row);
  });

  ui->torrentTabs->setCurrentIndex(0);
  ui->settingsTypeTabs->tabBar()->hide();
  ui->settingsTypeTabs->setCurrentIndex(0);

  QFont font = ui->orderListWidget->font();
  font.setCapitalization(QFont::Capitalize);
  ui->orderListWidget->setFont(font);

  loadSettings();
}

SettingsDialog::~SettingsDialog() {
  delete ui;
}

void SettingsDialog::accept() {
  applySettings();
  done(QDialog::Accepted);
}

void SettingsDialog::loadSettings() {
  Settings *s = new Settings;

  /* --- APPLICATION SETTINGS --- */

  // Startup
  bool start_on_boot = s->getValue(Settings::StartOnBoot, false).toBool();
  bool check_for_updates = s->getValue(Settings::CheckUpdates, true).toBool();
  bool start_minimized = s->getValue(Settings::StartMinimized, false).toBool();

  ui->startOnBootCheck->setChecked(start_on_boot);
  ui->checkforUpdatesCheck->setChecked(check_for_updates);
  ui->startMinimizedCheck->setChecked(start_minimized);

  // Update Settings
  QString update_stream =
      s->getValue(Settings::ReleaseStream, "Stable").toString();

  ui->updateStreamComboBox->setCurrentText(update_stream);

  // System Tray
  bool minimize_to_tray = s->getValue(Settings::MinimizeToTray, false).toBool();
  bool close_to_tray = s->getValue(Settings::CloseToTray, true).toBool();

  ui->minimizeToTrayCheck->setChecked(minimize_to_tray);
  ui->closeToTrayCheck->setChecked(close_to_tray);

  /* --- ANIME LIST --- */
  QStringList list_order =
      s->getValue(Settings::ListOrder, QStringList()).toStringList();
  ui->orderListWidget->addItems(list_order);

  /* --- RECOGNITION SETTINGS --- */

  // General Recognition Settings
  bool ear = s->getValue(Settings::AnimeRecognitionEnabled, true).toBool();
  ui->EARCheck->setChecked(ear);

  // Notification Settings
  bool detect_notify = s->getValue(Settings::AnimeDetectNotify, true).toBool();
  bool update_notify = s->getValue(Settings::AnimeUpdateNotify, true).toBool();

  ui->detectNotifyCheck->setChecked(detect_notify);
  ui->updateNotifyCheck->setChecked(update_notify);

  // Update Settings
  int update_delay = s->getValue(Settings::AutoUpdateDelay, 120).toInt();

  ui->updateDelaySpinBox->setValue(update_delay);

  /* --- TORRENT SETTINGS --- */

  // Automation
  bool enable_torrents = s->getValue(Settings::TorrentsEnabled, true).toBool();
  bool auto_download = s->getValue(Settings::AutoDownload, true).toBool();
  bool auto_notify = s->getValue(Settings::AutoNotify, false).toBool();

  ui->torrentCheck->setChecked(enable_torrents);
  ui->downloadRadio->setChecked(auto_download);
  ui->notifyRadio->setChecked(auto_notify);

  // Torrent Rules
  //loadTorrentRules();

  /* --- ADVANCED SETTINGS --- */

  // ...

  delete s;
}

void SettingsDialog::defaultSettings() {
  /* --- APPLICATION SETTINGS --- */

  // Startup
  ui->startOnBootCheck->setChecked(false);
  ui->checkforUpdatesCheck->setChecked(true);
  ui->startMinimizedCheck->setChecked(false);

  // Update Settings
  ui->updateStreamComboBox->setCurrentText("Stable");

  // System Tray
  ui->minimizeToTrayCheck->setChecked(false);
  ui->closeToTrayCheck->setChecked(true);

  /* --- ANIME LIST --- */
  ui->orderListWidget->clear();

  /* --- RECOGNITION SETTINGS --- */

  // General Recognition Settings
  ui->EARCheck->setChecked(true);

  // Notification Settings
  ui->detectNotifyCheck->setChecked(true);
  ui->updateNotifyCheck->setChecked(true);

  // Update Settings
  ui->updateDelaySpinBox->setValue(120);

  /* --- TORRENT SETTINGS --- */

  // Automation
  ui->torrentCheck->setChecked(true);
  ui->downloadRadio->setChecked(true);
  ui->notifyRadio->setChecked(false);

  // Torrent Rules
  //loadTorrentRules();

  /* --- ADVANCED SETTINGS --- */

  // ...

  applySettings();
}

void SettingsDialog::applySettings() {
  Settings *s = new Settings;

  /* --- APPLICATION SETTINGS --- */

  // Startup
  bool start_on_boot = ui->startOnBootCheck->isChecked();
  bool check_for_updates = ui->checkforUpdatesCheck->isChecked();
  bool start_minimized = ui->startMinimizedCheck->isChecked();

  s->setValue(Settings::StartOnBoot, start_on_boot);
  s->setValue(Settings::CheckUpdates, check_for_updates);
  s->setValue(Settings::StartMinimized, start_minimized);

  // Update Settings
  QString update_stream = ui->updateStreamComboBox->currentText();

  s->setValue(Settings::ReleaseStream, update_stream);

  // System Tray
  bool minimize_to_tray = ui->minimizeToTrayCheck->isChecked();
  bool close_to_tray = ui->closeToTrayCheck->isChecked();

  s->setValue(Settings::MinimizeToTray, minimize_to_tray);
  s->setValue(Settings::CloseToTray, close_to_tray);

  /* --- ANIME LIST --- */
  QStringList list_order;

  for(int i = 0; i < ui->orderListWidget->count(); i++) {
    list_order << ui->orderListWidget->item(i)->text();
  }

  s->setValue(Settings::ListOrder, list_order);

  /* --- RECOGNITION SETTINGS --- */

  // General Recognition Settings
  bool ear = ui->EARCheck->isChecked();
  s->setValue(Settings::AnimeRecognitionEnabled, ear);

  // Notification Settings
  bool detect_notify = ui->detectNotifyCheck->isChecked();
  bool update_notify = ui->updateNotifyCheck->isChecked();

  s->setValue(Settings::AnimeDetectNotify, detect_notify);
  s->setValue(Settings::AnimeUpdateNotify, update_notify);

  // Update Settings
  int update_delay = ui->updateDelaySpinBox->value();
  s->setValue(Settings::AutoUpdateDelay, update_delay);

  /* --- TORRENT SETTINGS --- */

  // Automation
  bool enable_torrents = ui->torrentCheck->isChecked();
  bool auto_download = ui->downloadRadio->isChecked();
  bool auto_notify = ui->notifyRadio->isChecked();

  s->setValue(Settings::TorrentsEnabled, enable_torrents);
  s->setValue(Settings::AutoDownload, auto_download);
  s->setValue(Settings::AutoNotify, auto_notify);

  // Torrent Rules
  //saveTorrentRules();

  /* --- ADVANCED SETTINGS --- */

  // ...

  delete s;
}
