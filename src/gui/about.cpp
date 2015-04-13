/* Copyright 2015 Kazakuri */

#include "./about.h"

#include <QFile>
#include <QDebug>
#include <QPainter>
#include <QCoreApplication>

#include "./ui_about.h"
#include "../lib/skinmanager.h"

About::About(QWidget *parent) : QDialog(parent), ui(new Ui::About) {
  qDebug() << "Launching about panel";
  ui->setupUi(this);
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

  QFile f(SkinManager::sharedManager()->get(SkinManager::AboutMascot));
  f.open(QFile::ReadOnly);
  QByteArray data = f.readAll();
  mascot.loadFromData(data);

  ui->versionLabel->setText(tr("Version ") + qApp->applicationVersion());
}

About::~About() {
  delete ui;
}

void About::paintEvent(QPaintEvent *e) {
  Q_UNUSED(e)

  QPainter p(this);

  p.drawPixmap(0, 0, width(), height(), mascot);
}
