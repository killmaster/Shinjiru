#include "rulemanager.h"
#include "ui_rulemanager.h"
#include "rulewizard.h"

#include <QDir>
#include <QDebug>
#include <QDirIterator>

RuleManager::RuleManager(QWidget *parent, QString default_rule) : QDialog(parent), ui(new Ui::RuleManager) {
  qDebug() << "Launching rule manager";
  ui->setupUi(this);
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
  setMinimumSize(250, 0);
  resize(250, height());

  QDir rule_dir(QCoreApplication::applicationDirPath() + "/rules/");
  rule_dir.setFilter(QDir::NoDotAndDotDot);
  if(!rule_dir.exists()) rule_dir.mkdir(".");

  QDirIterator dit(rule_dir.absolutePath(), QStringList() << "*.str", QDir::Files, QDirIterator::Subdirectories);

  while(dit.hasNext()) {
    QString file = dit.next().split("/").last();
    ui->listWidget->addItem(file);
  }

  qDebug() << ui->listWidget->count() << "rule files found";

  connect(ui->closeButton, &QPushButton::clicked, [&]() {
    this->accept();
  });

  connect(ui->deleteButton, &QPushButton::clicked, [&, rule_dir]() {
    qDebug() << "Deleted rule" << ui->listWidget->selectedItems().at(0)->text();
    if(ui->listWidget->selectedItems().length() == 0) return;
    QFile file(rule_dir.absolutePath()+ ui->listWidget->selectedItems().at(0)->text());
    file.remove();
    ui->listWidget->removeItemWidget(ui->listWidget->selectedItems().at(0));
  });

  connect(ui->newButton, &QPushButton::clicked, [&, default_rule]() {
    RuleWizard *rw = new RuleWizard(this, default_rule);
    rw->show();

    connect(rw, &RuleWizard::accepted, [&, rw]() {
      ui->listWidget->addItem(rw->fileName() + ".str");
    });
  });

  connect(ui->editButton, &QPushButton::clicked, [&, rule_dir]() {
    if(ui->listWidget->selectedItems().length() == 0) return;
    RuleWizard *rw = new RuleWizard(this, ui->listWidget->selectedItems().at(0)->text());
    rw->show();
  });
}

RuleManager::~RuleManager() {
  delete ui;
}
