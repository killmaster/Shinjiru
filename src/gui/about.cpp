#include "about.h"
#include "ui_about.h"
#include "../lib/skinmanager.h"

#include <QFile>
#include <QPainter>
#include <QCoreApplication>

About::About(QWidget *parent) : QDialog(parent), ui(new Ui::About) {
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    QFile f(SkinManager::sharedManager()->get(SkinManager::AboutMascot));
    f.open(QFile::ReadOnly);
    QByteArray data = f.readAll();
    mascot.loadFromData(data);

    ui->versionLabel->setText("Version " + qApp->applicationVersion());
}

About::~About()
{
    delete ui;
}

void About::paintEvent(QPaintEvent *) {
  QPainter p(this);

  p.drawPixmap(0, 0, width(), height(), mascot);
}
