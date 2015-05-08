#include "settingsdialog.h"
#include "ui_settingsdialog.h"

SettingsDialog::SettingsDialog(QWidget *parent) :
  QDialog(parent), ui(new Ui::SettingsDialog) {
  ui->setupUi(this);

  connect(ui->settingsTypeList, &QListWidget::currentRowChanged,
    [](int row) {  //NOLINT
    ui->settingsTypeTabs->setCurrentIndex(row);
  });
}

SettingsDialog::~SettingsDialog() {
  delete ui;
}
