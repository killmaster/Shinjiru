/* Copyright 2015 Kazakuri */

#include "./browseanime.h"
#include "./ui_browseanime.h"

#include "../api/anime.h"
#include "./scrolltext.h"
#include "./animepanel.h"

#include <QPainter>

BrowseAnime::BrowseAnime(QWidget *parent, int scoreType) : QWidget(parent), ui(new Ui::BrowseAnime) {
  ui->setupUi(this);

  QFont font = this->font();
  font.setPixelSize(13);
  setFont(font);

  this->scoreType = scoreType;

  text = new ScrollText(this);
  ui->verticalLayout->addWidget(text);
  ui->verticalLayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Minimum, QSizePolicy::Expanding));
}

BrowseAnime::~BrowseAnime() {
  delete ui;
}


void BrowseAnime::reload() {
  this->setAnime(this->anime);
  this->show();
}

void BrowseAnime::paintEvent(QPaintEvent *event) {
  QPainter p(this);

  QPen pen = p.pen();
  pen.setBrush(QBrush(QColor(255,255,255)));
  p.setPen(pen);

  p.drawPixmap(0, 0, width(), height(), anime->getCoverImage());
  p.fillRect  (0, 0, width(), 30, QBrush(QColor(0,0,0, 200)));

  event->accept();
}

void BrowseAnime::mouseDoubleClickEvent(QMouseEvent *event) {
  AnimePanel *ap = new AnimePanel(this, anime, scoreType);
  ap->show();
  event->accept();
}

void BrowseAnime::setAnime(Anime *anime) {
  this->anime = anime;

  text->setText(anime->getTitle());
}

Anime* BrowseAnime::getAnime() {
  return this->anime;
}
