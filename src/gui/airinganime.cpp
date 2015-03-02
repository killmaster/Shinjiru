#include "airinganime.h"
#include "ui_airinganime.h"

#include "../api/anime.h"
#include "scrolltext.h"
#include "animepanel.h"

#include <QPainter>

AiringAnime::AiringAnime(QWidget *parent, int scoreType) : QWidget(parent), ui(new Ui::AiringAnime) {
  ui->setupUi(this);

  QFont font = this->font();
  font.setPixelSize(13);
  setFont(font);

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

  QPen pen = p.pen();
  pen.setBrush(QBrush(QColor(255,255,255)));
  p.setPen(pen);

  p.drawPixmap(0, 0, width(), height(), anime->getCoverImage());
  p.fillRect  (0, 0, width(), 30, QBrush(QColor(0,0,0, 200)));

  p.fillRect(0, height() - 20, width(), 20, QBrush(QColor(0,0,0, 200)));
  p.drawText(QRect(5, height() - 20, width() - 5, 20), Qt::AlignLeft, "Ep. " + QString::number(anime->getNextEpisode()));

  QString cd = "";

  int countdown = anime->getCountdown();
  countdown /= 3600; //convert to hours

  if(countdown > 24) {
    countdown /= 24; //convert to days
    cd = QString::number(countdown) + " day" + (countdown != 1 ? "s" : "");
  } else {
    if(countdown == 0) cd = "<1 hour";
    else cd = QString::number(countdown) + " hour" + (countdown != 1 ? "s" : "");
  }

  p.drawText(QRect(0, height() - 20, width() - 5, 20), Qt::AlignRight, cd);

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

void AiringAnime::tick() {
  int c = anime->getCountdown();
  if(c == 0) {
    c = 604801; // TODO: This is bad, defaults to a week which isn't always correct
    anime->setNextEpisode(anime->getNextEpisode() + 1);
  }

  c--;

  anime->setCountdown(c);
  repaint();
}
