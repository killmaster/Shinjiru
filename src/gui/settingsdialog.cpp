#include "./settingsdialog.h"
#include "./ui_settingsdialog.h"

#include <QDesktopServices>
#include <QUrl>
#include <QDir>

#include "../settings.h"
#include "../lib/skinmanager.h"
#include "../api/anime.h"
#include "../api/user.h"

#ifdef Q_OS_WIN
  const QString winkey =
      "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run";
#endif

const QChar sp = QChar(0x202F);
const QString seperator = QString(sp + QString(" - "));

SettingsDialog::SettingsDialog(QWidget *parent) :
  QDialog(parent), ui(new Ui::SettingsDialog) {
  ui->setupUi(this);
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

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
  connect(ui->disconnectButton, SIGNAL(clicked()), SLOT(resetAPI()));
  connect(ui->defaultButton, SIGNAL(clicked()), SLOT(defaultSettings()));
  connect(ui->openSkinsFolderButton, &QPushButton::clicked, [&]() {  // NOLINT
    QDesktopServices::openUrl(QUrl(qApp->applicationDirPath() + "/data/skin/"));
  });
  connect(ui->newTitle, &QPushButton::clicked, [&]() {  // NOLINT
    ui->smartTitleList->addItem("**New**");
    ui->smartTitleList->setCurrentRow(ui->smartTitleList->count() - 1);
  });

  connect(ui->deleteTitle, &QPushButton::clicked, [&](){  // NOLINT
    QListWidgetItem *current = ui->smartTitleList->currentItem();

    if (current != nullptr) {
      int row = ui->smartTitleList->row(current);
      delete ui->smartTitleList->currentItem();

      ui->smartTitleList->setCurrentRow(qMax(row - 1, 0));
    }
  });
  connect(ui->aliasLineEdit, SIGNAL(textEdited(QString)),
          SLOT(updateSmartTitleName()));
  connect(ui->titleComboBox, SIGNAL(currentIndexChanged(int)),
          SLOT(updateSmartTitleName()));
  connect(ui->offsetSpinBox, SIGNAL(valueChanged(int)), SLOT(updateSmartTitleName()));

  connect(ui->smartTitleList, &QListWidget::currentItemChanged, [&]() {  // NOLINT
    if (ui->smartTitleList->currentItem() == nullptr) return;

    QStringList text = ui->smartTitleList->currentItem()->text().split(seperator);

    ui->aliasLineEdit->setText(text.at(0));

    ui->offsetSpinBox->setValue(text.last().toInt());

    if (text.length() > 2) {
      ui->titleComboBox->disconnect(this);
      ui->titleComboBox->setCurrentText(text.at(1) + seperator + text.at(2));
      connect(ui->titleComboBox, SIGNAL(currentIndexChanged(int)),
              SLOT(updateSmartTitleName()));
    }

    if (text.length() > 3)
      ui->offsetSpinBox->setValue(text.at(3).toInt());
  });

  connect(ui->torrentRuleList, &QListWidget::currentItemChanged, [&]() {  // NOLINT
    if(!current_rule.isEmpty()) {
      torrent_rules.remove(current_rule);
      QListWidgetItem *item =
          ui->torrentRuleList->findItems(current_rule, Qt::MatchExactly).at(0);
      int index = ui->torrentRuleList->row(item);

      delete item;

      QJsonObject new_rule;
      QString key;

      if(ui->basicBox->isChecked()) {
        key = ui->animeTitleLineEdit->text();

        new_rule.insert("rule_type", "basic");
        new_rule.insert("anime", ui->animeTitleLineEdit->text());
        new_rule.insert("subgroup", ui->subGroupLineEdit->text());
        new_rule.insert("resolution", ui->animeResolutionComboBox->currentText());
      } else {
        key = ui->fileRegexLineEdit->text();

        new_rule.insert("rule_type", "advanced");
        new_rule.insert("regex", ui->fileRegexLineEdit->text());
      }

      torrent_rules.insert(key, new_rule);
      ui->torrentRuleList->insertItem(index, key);
    }

    QString key = ui->torrentRuleList->currentItem()->text();
    QJsonObject rule = torrent_rules.value(key).toObject();

    QString rule_type = rule.value("rule_type").toString();

    if(rule_type == "basic") {
      ui->basicBox->setChecked(true);
      ui->advancedBox->setChecked(false);

      ui->animeTitleLineEdit->setText(rule.value("anime").toString());
      ui->subGroupLineEdit->setText(rule.value("subgroup").toString());
      ui->animeResolutionComboBox->setCurrentText(rule.value("resolution").toString());
    } else {
      ui->basicBox->setChecked(true);
      ui->advancedBox->setChecked(false);

      ui->fileRegexLineEdit->setText(rule.value("regex").toString());
    }

    current_rule = key;
  });

  connect(ui->animeTitleLineEdit, &QLineEdit::textEdited, [&]() {
    ui->torrentRuleList->currentItem()->setText(ui->animeTitleLineEdit->text());
  });

  connect(ui->fileRegexLineEdit, &QLineEdit::textEdited, [&]() {
    ui->torrentRuleList->currentItem()->setText(ui->fileRegexLineEdit->text());
  });

  connect(ui->basicBox, SIGNAL(toggled(bool)), SLOT(toggleBasic(bool)));
  connect(ui->advancedBox, SIGNAL(toggled(bool)), SLOT(toggleAdvanced(bool)));

  ui->torrentTabs->setCurrentIndex(0);
  ui->settingsTypeTabs->tabBar()->hide();
  ui->settingsTypeTabs->setCurrentIndex(0);
  ui->smartTitleList->setCurrentRow(0);
  updateSmartTitleName();

  awesome = new QtAwesome(qApp);
  awesome->initFontAwesome();

  QVariantMap black;
  black.insert("color", QColor(0, 0, 0));
  black.insert("color-active", QColor(0, 0, 0));
  black.insert("color-disabled", QColor(0, 0, 0));
  black.insert("color-selected", QColor(0, 0, 0));

  ui->moveDownButton->setIcon(awesome->icon(fa::arrowdown, black));
  ui->moveUpButton->setIcon(awesome->icon(fa::arrowup, black));
  ui->moveUpButton->setText("");
  ui->moveDownButton->setText("");

  connect(ui->moveUpButton, SIGNAL(clicked()), SLOT(moveUp()));
  connect(ui->moveDownButton, SIGNAL(clicked()), SLOT(moveDown()));

  QFont font = ui->orderListWidget->font();
  font.setCapitalization(QFont::Capitalize);
  ui->orderListWidget->setFont(font);

  loadSettings();
}

SettingsDialog::~SettingsDialog() {
  delete ui;
  delete awesome;
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


  if (start_on_boot) {
    #ifdef Q_OS_WIN
      QSettings reg(winkey, QSettings::NativeFormat);
      QString path = reg.value("Shinjiru", QString("")).toString();
      if (path.isEmpty()) start_on_boot = false;
    #endif
  }

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

  // Smart Titles
  loadSmartTitles();

  /* --- TORRENT SETTINGS --- */

  // Automation
  bool enable_torrents = s->getValue(Settings::TorrentsEnabled, true).toBool();
  int refresh_interval = s->getValue(Settings::TorrentRefreshTime, 900).toInt();
  bool auto_download = s->getValue(Settings::AutoDownload, true).toBool();
  bool auto_notify = s->getValue(Settings::AutoNotify, false).toBool();

  ui->torrentCheck->setChecked(enable_torrents);
  ui->refreshIntervalSpinBox->setValue(refresh_interval);
  ui->downloadRadio->setChecked(auto_download);
  ui->notifyRadio->setChecked(auto_notify);

  // Torrent Rules
  loadTorrentRules();

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

  // Smart Titles
  //loadSmartTitles();

  /* --- TORRENT SETTINGS --- */

  // Automation
  ui->torrentCheck->setChecked(true);
  ui->refreshIntervalSpinBox->setValue(900);
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


  if (start_on_boot) {
    #ifdef Q_OS_WIN
      QSettings reg(winkey, QSettings::NativeFormat);
      reg.setValue("Shinjiru", "\"" +
                   qApp->applicationFilePath().replace("/", "\\") + "\"");
    #endif
  } else {
    #ifdef Q_OS_WIN
      QSettings reg(winkey, QSettings::NativeFormat);
      reg.remove("Shinjiru");
    #endif
  }

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

  // Smart Titles
  saveSmartTitles();

  /* --- TORRENT SETTINGS --- */

  // Automation
  bool enable_torrents = ui->torrentCheck->isChecked();
  int refresh_interval = ui->refreshIntervalSpinBox->value();
  bool auto_download = ui->downloadRadio->isChecked();
  bool auto_notify = ui->notifyRadio->isChecked();

  s->setValue(Settings::TorrentsEnabled, enable_torrents);
  s->setValue(Settings::TorrentRefreshTime, refresh_interval);
  s->setValue(Settings::AutoDownload, auto_download);
  s->setValue(Settings::AutoNotify, auto_notify);

  // Torrent Rules
  saveTorrentRules();

  /* --- ADVANCED SETTINGS --- */

  // ...

  delete s;
}


void SettingsDialog::moveUp() {
  if (ui->orderListWidget->selectedItems().count() == 1) {
    int row =
       ui->orderListWidget->row(ui->orderListWidget->selectedItems().at(0));
    if (row != 0) {
      ui->orderListWidget->insertItem(
           row - 1, ui->orderListWidget->takeItem(row)->text());
      ui->orderListWidget->setCurrentRow(row - 1);
    }
  }
}

void SettingsDialog::moveDown() {
  if (ui->orderListWidget->selectedItems().count() == 1) {
    int row =
        ui->orderListWidget->row(ui->orderListWidget->selectedItems().at(0));
    if (row != ui->orderListWidget->count()) {
      ui->orderListWidget->insertItem(
            row + 1, ui->orderListWidget->takeItem(row)->text());
      ui->orderListWidget->setCurrentRow(row + 1);
    }
  }
}

void SettingsDialog::loadTorrentRules() {
  QFile tor_rule_file(QCoreApplication::applicationDirPath() + "/rules.json");
  tor_rule_file.open(QFile::ReadWrite);

  torrent_rules = QJsonDocument::fromJson(tor_rule_file.readAll()).object();

  ui->torrentRuleList->addItems(torrent_rules.keys());
}

void SettingsDialog::saveTorrentRules() {
  if(!current_rule.isEmpty()) {
    torrent_rules.remove(current_rule);
    QListWidgetItem *item =
        ui->torrentRuleList->findItems(current_rule, Qt::MatchExactly).at(0);
    int index = ui->torrentRuleList->row(item);

    delete item;

    QJsonObject new_rule;
    QString key;

    if(ui->basicBox->isChecked()) {
      key = ui->animeTitleLineEdit->text();

      new_rule.insert("rule_type", "basic");
      new_rule.insert("anime", ui->animeTitleLineEdit->text());
      new_rule.insert("subgroup", ui->subGroupLineEdit->text());
      new_rule.insert("resolution", ui->animeResolutionComboBox->currentText());
    } else {
      key = ui->fileRegexLineEdit->text();

      new_rule.insert("rule_type", "advanced");
      new_rule.insert("regex", ui->fileRegexLineEdit->text());
    }

    torrent_rules.insert(key, new_rule);
    ui->torrentRuleList->insertItem(index, key);
  }

  QFile tor_rule_file(QCoreApplication::applicationDirPath() + "/rules.json");
  tor_rule_file.open(QFile::WriteOnly);

  tor_rule_file.write(QJsonDocument(torrent_rules).toJson());
}

void SettingsDialog::loadSmartTitles() {
  QList<Anime *> list = User::sharedUser()->getAnimeList();

  qSort(list.begin(), list.end(), [](Anime *&s1, Anime*&s2) {  // NOLINT
    return s1->getTitle() < s2->getTitle();
  });

  for (Anime *a : list) {
    ui->titleComboBox->addItem(a->getTitle() + seperator + a->getID());
  }

  QFile smart_file(QCoreApplication::applicationDirPath() + "/relations.json");
  if (!smart_file.open(QFile::ReadOnly)) return;

  QJsonArray relations = QJsonDocument::fromJson(smart_file.readAll()).array();

  for (QJsonValue v : relations) {
    QJsonObject relation = v.toObject();

    QString id = relation.value("id").toString("0");
    QString custom = relation.value("custom").toString();
    QString title = relation.value("title").toString();
    int offset = relation.value("offset").toString().toInt(0);

    ui->smartTitleList->addItem(custom + seperator +
                                title + seperator +
                                id + seperator +
                                QString::number(offset));
  }
}

void SettingsDialog::saveSmartTitles() {
  QFile f(QApplication::applicationDirPath() + "/relations.json");
  f.open(QFile::WriteOnly);

  QJsonArray arr;

  for (int i = 0; i < ui->smartTitleList->count(); i++) {
    QStringList data = ui->smartTitleList->item(i)->text().split(seperator);

    if (data.length() < 3) continue;

    QJsonObject o;

    o.insert("id", data.at(2));
    o.insert("title", data.at(1));
    o.insert("custom", data.at(0));
    o.insert("offset", data.at(3));

    arr.append(o);
  }

  f.write(QJsonDocument(arr).toJson());
}
void SettingsDialog::updateSmartTitleName() {
  QString lineText = ui->aliasLineEdit->text() + seperator +
                     ui->titleComboBox->currentText() + seperator +
                     QString::number(ui->offsetSpinBox->value());

  if (ui->smartTitleList->currentItem() != 0)
    ui->smartTitleList->currentItem()->setText(lineText);
}

void SettingsDialog::showSmartTitles() {
  this->show();
  ui->settingsTypeTabs->setCurrentIndex(2);
  ui->settingsTypeList->setCurrentRow(2);
  ui->recognitionTab->setCurrentIndex(1);

  if(ui->smartTitleList->count() > 0)
    ui->smartTitleList->setCurrentRow(0);
}


void SettingsDialog::toggleBasic(bool en) {
  ui->basicBox->setChecked(en);
  ui->advancedBox->setChecked(!en);
}

void SettingsDialog::toggleAdvanced(bool en) {
  ui->basicBox->setChecked(!en);
  ui->advancedBox->setChecked(en);
}
