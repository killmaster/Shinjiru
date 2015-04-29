/* Copyright 2015 Kazakuri */

#include "./smarttitlemanager.h"

#include <QLineEdit>
#include <QComboBox>
#include <QFile>

#include <ui_smarttitlemanager.h>

#include "../api/user.h"
#include "../api/anime.h"
#include "../api/smarttitle.h"

const QChar sp = QChar(0x202F);
const QString seperator = QString(sp + QString(" - "));

bool animeSort(Anime *&s1, Anime *&s2) {  // NOLINT
  return s1->getTitle() < s2->getTitle();
}

SmartTitleManager::SmartTitleManager(QWidget *parent) :
  QDialog(parent), ui(new Ui::SmartTitleManager) {
  ui->setupUi(this);

  QList<Anime *> list = User::sharedUser()->getAnimeList();

  qSort(list.begin(), list.end(), animeSort);

  for (Anime *a : list) {
    ui->comboBox->addItem(a->getTitle() + seperator + a->getID());
  }

  QList<SmartTitle *> smart_titles = User::sharedUser()->getSmartTitles();

  for (SmartTitle *s : smart_titles) {
    ui->listWidget->addItem(s->getCustom() + seperator +
                            s->getTitle() + seperator +
                            s->getID() + seperator +
                            QString::number(s->getOffset()));

    qDebug() << s->getOffset();
  }

  connect(ui->newTitle, &QPushButton::clicked, [&]() {  // NOLINT
    ui->listWidget->addItem("**New**");
    ui->listWidget->setCurrentRow(ui->listWidget->count() - 1);
  });

  connect(ui->lineEdit, SIGNAL(textEdited(QString)), SLOT(updateName()));
  connect(ui->comboBox, SIGNAL(currentIndexChanged(int)),
          SLOT(updateName()));
  connect(ui->spinBox, SIGNAL(valueChanged(int)), SLOT(updateName()));

  connect(ui->listWidget, &QListWidget::currentItemChanged, [&]() {  // NOLINT
    QStringList text = ui->listWidget->currentItem()->text().split(seperator);

    ui->lineEdit->setText(text.at(0));

    ui->spinBox->setValue(text.last().toInt());

    if (text.length() > 2)
      ui->comboBox->setCurrentText(text.at(1) + seperator + text.at(2));

    if (text.length() > 3)
      ui->spinBox->setValue(text.at(3).toInt());
  });

  ui->listWidget->setCurrentRow(0);
  updateName();
}

SmartTitleManager::~SmartTitleManager() {
  delete ui;
}

void SmartTitleManager::updateName() {
  QString lineText = ui->lineEdit->text() + seperator +
                     ui->comboBox->currentText() + seperator +
                     QString::number(ui->spinBox->value());

  if (ui->listWidget->currentItem() != 0)
    ui->listWidget->currentItem()->setText(lineText);
}

void SmartTitleManager::accept() {
  QFile f(QApplication::applicationDirPath() + "/relations.json");
  f.open(QFile::WriteOnly);

  QJsonArray arr;

  for (int i = 0; i < ui->listWidget->count(); i++) {
    QStringList data = ui->listWidget->item(i)->text().split(seperator);

    if (data.length() < 3) continue;

    QJsonObject o;

    o.insert("id", data.at(2));
    o.insert("title", data.at(1));
    o.insert("custom", data.at(0));
    o.insert("offset", data.at(3));

    arr.append(o);
  }

  f.write(QJsonDocument(arr).toJson());

  done(QDialog::Accepted);
}
