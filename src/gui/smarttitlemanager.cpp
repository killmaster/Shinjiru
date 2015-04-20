/* Copyright 2015 Kazakuri */

#include "./smarttitlemanager.h"

#include <QLineEdit>
#include <QComboBox>
#include <QFile>

#include <ui_smarttitlemanager.h>

#include "../api/user.h"
#include "../api/anime.h"
#include "../api/smarttitle.h"

bool animeSort(Anime *&s1, Anime *&s2) {  // NOLINT
  return s1->getTitle() < s2->getTitle();
}

SmartTitleManager::SmartTitleManager(QWidget *parent) :
  QDialog(parent), ui(new Ui::SmartTitleManager) {
  ui->setupUi(this);

  QList<Anime *> list = User::sharedUser()->getAnimeList();

  qSort(list.begin(), list.end(), animeSort);

  for (Anime *a : list) {
    ui->comboBox->addItem(a->getTitle() + " - " + a->getID());
  }

  QList<SmartTitle *> smart_titles = User::sharedUser()->getSmartTitles();

  for (SmartTitle *s : smart_titles) {
    ui->listWidget->addItem(s->getCustom() + " - " +
                            s->getID() + " - " +
                            s->getTitle());
  }

  connect(ui->newTitle, &QPushButton::clicked, [&]() {  // NOLINT
    ui->listWidget->addItem("**New**");
    ui->listWidget->setCurrentRow(ui->listWidget->count() - 1);
  });

  connect(ui->lineEdit, SIGNAL(textChanged(QString)), SLOT(updateName()));
  connect(ui->comboBox, SIGNAL(currentIndexChanged(int)),
          SLOT(updateName()));

  connect(ui->listWidget, &QListWidget::currentItemChanged, [&]() {  // NOLINT
    QStringList text = ui->listWidget->currentItem()->text().split(" - ");

    ui->lineEdit->setText(text.at(0));

    if (text.length() > 2)
      ui->comboBox->setCurrentText(text.at(1) + " - " + text.at(2));
    else if (text.length() > 1)
      ui->comboBox->setCurrentText(text.at(1));
  });

  ui->listWidget->setCurrentRow(0);
  updateName();
}

SmartTitleManager::~SmartTitleManager() {
  delete ui;
}

void SmartTitleManager::updateName() {
  QString lineText = ui->lineEdit->text() + " - " +
                     ui->comboBox->currentText();

  if (ui->listWidget->currentItem() != 0)
    ui->listWidget->currentItem()->setText(lineText);
}

void SmartTitleManager::accept() {
  QFile f(QApplication::applicationDirPath() + "/relations.json");
  f.open(QFile::WriteOnly);

  QJsonArray arr;

  for (int i = 0; i < ui->listWidget->count(); i++) {
    QStringList data = ui->listWidget->item(i)->text().split(" - ");

    if (data.length() < 3) continue;

    QJsonObject o;

    o.insert("id", data.at(1));
    o.insert("title", data.at(2));
    o.insert("custom", data.at(0));

    arr.append(o);
  }

  f.write(QJsonDocument(arr).toJson());

  done(QDialog::Accepted);
}
