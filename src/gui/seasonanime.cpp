#include "SeasonAnime.h"
#include "ui_SeasonAnime.h"

#include "../api/anime.h"
#include "scrolltext.h"
#include "animepanel.h"

#include <QPainter>

SeasonAnime::SeasonAnime(QWidget *parent, int scoreType) : QWidget(parent), ui(new Ui::SeasonAnime) {
  ui->setupUi(this);

  QFont font = this->font();
  font.setPixelSize(13);
  setFont(font);

  this->scoreType = scoreType;

  text = new ScrollText(this);
  ui->verticalLayout->addWidget(text);
  ui->verticalLayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Minimum, QSizePolicy::Expanding));
}

SeasonAnime::~SeasonAnime() {
  delete ui;
}

void SeasonAnime::paintEvent(QPaintEvent *event) {
  QPainter p(this);

  QPen pen = p.pen();
  pen.setBrush(QBrush(QColor(255,255,255)));
  p.setPen(pen);

  p.drawPixmap(0, 0, width(), height(), anime->getCoverImage());
  p.fillRect  (0, 0, width(), 30, QBrush(QColor(0,0,0, 200)));

  event->accept();
}

void SeasonAnime::mouseDoubleClickEvent(QMouseEvent *event) {
  AnimePanel *ap = new AnimePanel(this, anime, scoreType);
  ap->show();
  event->accept();
}

void SeasonAnime::setAnime(Anime *anime) {
  this->anime = anime;

  text->setText(anime->getTitle());

  if(anime->needsLoad()) {
    connect(anime, SIGNAL(new_image()), SLOT(repaint()));
  }
}

Anime* SeasonAnime::getAnime() {
  return this->anime;
}
