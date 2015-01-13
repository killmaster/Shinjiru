#include "airinganime.h"
#include "ui_airinganime.h"

#include "../api/anime.h"
#include "scrolltext.h"
#include "animepanel.h"

#include <QPainter>

AiringAnime::AiringAnime(QWidget *parent, int scoreType) : QWidget(parent), ui(new Ui::AiringAnime) {
  ui->setupUi(this);

  this->scoreType = scoreType;

  text = new ScrollText(this);
  ui->verticalLayout->addWidget(text);
  ui->verticalLayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Minimum, QSizePolicy::Expanding));
}

AiringAnime::~AiringAnime() {
  delete ui;
}

void AiringAnime::paintEvent(QPaintEvent *event) {
  QPainter p(this);

  p.drawPixmap(0, 0, width(), height(), anime->getCoverImage());
  p.fillRect  (0, 0, width(), 30, QBrush(QColor(0,0,0, 200)));

  event->accept();
}

void AiringAnime::mouseDoubleClickEvent(QMouseEvent *event) {
  AnimePanel *ap = new AnimePanel(this, anime, scoreType);
  ap->show();
  event->accept();
}

void AiringAnime::setAnime(Anime *anime) {
  this->anime = anime;

  text->setText(anime->getTitle());

  if(anime->needsLoad()) {
    connect(anime, SIGNAL(new_image()), SLOT(repaint()));
  }
}

Anime* AiringAnime::getAnime() {
    return this->anime;
}
