#include "rulewizard.h"
#include "ui_rulewizard.h"

RuleWizard::RuleWizard(QWidget *parent, QString title, QString sub, QString res, QString file) : QDialog(parent), ui(new Ui::RuleWizard) {
  ui->setupUi(this);

  ui->animeTitleLineEdit->setText(title);
  ui->subGroupLineEdit->setText(sub);
  ui->animeResolutionComboBox->setCurrentText(res);
  ui->fileRegexLineEdit->setText(file);

  connect(ui->groupBox,   SIGNAL(toggled(bool)), SLOT(groupToggle(bool)));
  connect(ui->groupBox_2, SIGNAL(toggled(bool)), SLOT(group2Toggle(bool)));
}

RuleWizard::~RuleWizard() {
  delete ui;
}

void RuleWizard::groupToggle(bool status) {
  ui->groupBox_2->setChecked(!status);
}

void RuleWizard::group2Toggle(bool status) {
  ui->groupBox->setChecked(!status);
}
