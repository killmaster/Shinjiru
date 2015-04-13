/* Copyright 2015 Kazakuri */

#include "./rulewizard.h"

#include <QInputDialog>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QDate>

#include "./ui_rulewizard.h"

RuleWizard::RuleWizard(QWidget *parent, QString title,
                       QString sub, QString res, QString file,
                       QString default_rule) :
  QDialog(parent), ui(new Ui::RuleWizard) {
  qDebug() << "Launching rule wizard";
  ui->setupUi(this);
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

  ui->lineEdit->setValidator(new QIntValidator(0, 999, this));

  if (default_rule == "basic") {
      ui->groupBox->setChecked(false);
      ui->groupBox_2->setChecked(true);
  } else {
      ui->groupBox->setChecked(true);
      ui->groupBox_2->setChecked(false);
  }

  ui->animeTitleLineEdit->setText(title);
  ui->subGroupLineEdit->setText(sub);
  ui->animeResolutionComboBox->setCurrentText(res);
  ui->fileRegexLineEdit->setText(file);

  edit_mode = false;

  connect(ui->groupBox,   SIGNAL(toggled(bool)), SLOT(groupToggle(bool)));
  connect(ui->groupBox_2, SIGNAL(toggled(bool)), SLOT(group2Toggle(bool)));
}

RuleWizard::RuleWizard(QWidget *parent, QString file) :
  QDialog(parent), ui(new Ui::RuleWizard) {
  if (file == "basic" || file == "advanced") {
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    ui->lineEdit->setValidator(new QIntValidator(0, 999, this));

    if (file == "basic") {
        ui->groupBox->setChecked(false);
        ui->groupBox_2->setChecked(true);
    } else {
        ui->groupBox->setChecked(true);
        ui->groupBox_2->setChecked(false);
    }

    connect(ui->groupBox,   SIGNAL(toggled(bool)), SLOT(groupToggle(bool)));
    connect(ui->groupBox_2, SIGNAL(toggled(bool)), SLOT(group2Toggle(bool)));

    edit_mode = false;

    return;
  }

  QFile f(qApp->applicationDirPath() + "/rules/" + file);
  f.open(QFile::ReadOnly);

  QJsonObject o = QJsonDocument::fromJson(f.readAll()).object();

  QString type = o["rule_type"].toString();


  edit_mode = true;
  file_name = file.split("/").last().split(".").first();

  if (type == "basic") {
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    ui->groupBox->setChecked(false);
    ui->groupBox_2->setChecked(true);

    ui->animeTitleLineEdit->setText(o["anime_name"].toString());
    ui->subGroupLineEdit->setText(o["sub_group"].toString());
    ui->animeResolutionComboBox->setCurrentText(o["resolution"].toString());

    connect(ui->groupBox,   SIGNAL(toggled(bool)), SLOT(groupToggle(bool)));
    connect(ui->groupBox_2, SIGNAL(toggled(bool)), SLOT(group2Toggle(bool)));
  } else {
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    ui->groupBox->setChecked(true);
    ui->groupBox_2->setChecked(false);

    ui->fileRegexLineEdit->setText(o["file_regex"].toString());

    connect(ui->groupBox,   SIGNAL(toggled(bool)), SLOT(groupToggle(bool)));
    connect(ui->groupBox_2, SIGNAL(toggled(bool)), SLOT(group2Toggle(bool)));
  }

  ui->lineEdit->setValidator(new QIntValidator(0, 999, this));
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
  if (ui->lineEdit->text().isEmpty()) {
    ui->lineEdit->setText("-1");
  }

  if (!edit_mode){
    bool ok;
    file_name =
        QInputDialog::getText(static_cast<QWidget *>(this->parent()),
                              tr("Rule Name"),
                              tr("Enter a name for the new rule:"),
                              QLineEdit::Normal, "", &ok);
  }

  qDebug() << "Creating rule" << file_name + ".str";

  QDir rule_dir(QCoreApplication::applicationDirPath() + "/rules/");
  if (!rule_dir.exists()) rule_dir.mkdir(".");
  QFile file(rule_dir.absolutePath() + "/" + file_name + ".str");
  QJsonObject rule_json;

  if (ui->groupBox->isChecked()) {
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

  int expires = ui->lineEdit->text().toInt();

  QDate date = expires < 0 ? QDate::currentDate().addYears(999) :
                             QDate::currentDate().addDays(7 * expires);


  rule_json["expires"] = date.toString();

  file.open(QFile::WriteOnly);
  QByteArray data = QJsonDocument(rule_json).toJson(QJsonDocument::Compact);
  file.write(data);
  file.close();
  done(QDialog::Accepted);
}
