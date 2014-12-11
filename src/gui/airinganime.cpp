#include "airinganime.h"
#include "ui_airinganime.h"

#include "../api/anime.h"
#include "scrolltext.h"

#include <QPainter>

AiringAnime::AiringAnime(QWidget *parent) : QWidget(parent), ui(new Ui::AiringAnime) {
  ui->setupUi(this);

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

void AiringAnime::setAnime(Anime *anime) {
  this->anime = anime;

  text->setText(anime->getRomajiTitle());

  if(anime->needsLoad()) {
    connect(anime, SIGNAL(new_image()), SLOT(repaint()));
  }
}
