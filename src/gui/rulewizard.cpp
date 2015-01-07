#include "rulewizard.h"
#include "ui_rulewizard.h"

#include <QInputDialog>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QDir>

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

void RuleWizard::accept() {
  bool ok;
  QString name = QInputDialog::getText(static_cast<QWidget *>(this->parent()), tr("Rule Name"), tr("Enter a name for the new rule:"), QLineEdit::Normal, "", &ok);
  QDir rule_dir(QCoreApplication::applicationDirPath() + "/rules/");
  if(!rule_dir.exists()) rule_dir.mkdir(".");
  QFile file(rule_dir.absolutePath() + "/" + name + ".str");
  QJsonObject rule_json;

  if(ui->groupBox->isChecked()) {
    // Advanced group box
    rule_json["rule_type"]  = "advanced";
    rule_json["file_regex"] = ui->fileRegexLineEdit->text();
  } else {
    // Basic group box
    rule_json["rule_type"]  = "basic";
    rule_json["anime_name"] = ui->animeTitleLineEdit->text();
    rule_json["sub_group"]  = ui->subGroupLineEdit->text();
    rule_json["resolution"] = ui->animeResolutionComboBox->currentText();
  }
  file.open(QFile::WriteOnly);
  QByteArray data = QJsonDocument(rule_json).toJson(QJsonDocument::Compact);
  file.write(data);
  file.close();
  done(QDialog::Accepted);
}