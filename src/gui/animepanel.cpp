#include "animepanel.h"
#include "ui_animepanel.h"
#include "mainwindow.h"

#include <QDebug>

AnimePanel::AnimePanel(QWidget *parent, Anime *anime) : QDialog(parent), ui(new Ui::AnimePanel) {
  ui->setupUi(this);
  setAttribute(Qt::WA_DeleteOnClose);

  this->anime = anime;
  MainWindow *par = static_cast<MainWindow *>(parent);
  int score_type = 0;//par->scoreType();
  QString airing_status = anime->getAiringStatus();
  airing_status = airing_status.at(0).toUpper() + airing_status.right(airing_status.length() - 1);

  ui->lblTitle->setText(anime->getRomajiTitle());
  ui->txtType->setText(anime->getType());
  ui->txtEpisodes->setText(QString::number(anime->getEpisodeCount()));
  ui->txtScore->setText(anime->getAverageScore());
  ui->txtAiring->setText(airing_status);

  ui->spinEps->setMaximum(anime->getEpisodeCount());
  ui->spinEps->setValue(anime->getMyProgress());
  ui->txtcore->setText(anime->getMyScore());
  ui->txtNotes->setText(anime->getMyNotes());
  ui->spinRewatch->setValue(anime->getMyRewatch());
  ui->comboStatus->setCurrentText(anime->getMyStatus());

  if(anime->needsLoad()) {
    loadNeeded = true;
    connect(anime, SIGNAL(finishedReloading()), SLOT(refreshDisplay()));
    connect(anime, SIGNAL(new_image()), SLOT(repaint()));
  } else {
    refreshDisplay();
  }
}

AnimePanel::~AnimePanel() {
  delete ui;
}

void AnimePanel::paintEvent(QPaintEvent *event) {
  QPainter p(this);

  QPixmap cover = anime->getCoverImage();
  QMargins m = ui->detailForm->contentsMargins();
  m.setRight(20 + cover.width());
  ui->detailForm->setContentsMargins(m);

  p.drawPixmap(width() - 235, 10, cover.width(), cover.height(), cover);
  p.drawRect  (width() - 235, 10, cover.width(), cover.height());

  event->accept();
}

void AnimePanel::refreshDisplay() {
  QString synonyms = anime->getSynonyms().join(", ");
  if(synonyms == "") synonyms = "None";
  ui->lblSynonyms->setText(synonyms);
  ui->txtSynopsis->setText(anime->getSynopsis());
}
