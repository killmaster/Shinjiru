#include "animepanel.h"
#include "ui_animepanel.h"

#include <QDebug>

AnimePanel::AnimePanel(QWidget *parent, Anime *anime) : QDialog(parent), ui(new Ui::AnimePanel) {
  ui->setupUi(this);
  setAttribute(Qt::WA_DeleteOnClose);

  this->anime = anime;

  ui->lblTitle->setText(anime->getRomajiTitle());
  ui->txtType->setText(anime->getType());
  ui->txtEpisodes->setText(QString::number(anime->getEpisodeCount()));
  ui->txtScore->setText(anime->getAverageScore());
  ui->txtAiring->setText(anime->getAiringStatus());

  if(anime->needsLoad()) {
    loadNeeded = true;
    connect(anime, SIGNAL(finishedReloading()), SLOT(refreshDisplay()));
  } else {
    refreshDisplay();
  }
}

AnimePanel::~AnimePanel() {
  delete ui;
}

void AnimePanel::refreshDisplay() {
  QString synonyms = anime->getSynonyms().join(", ");
  if(synonyms == "") synonyms = "None";
  ui->lblSynonyms->setText(synonyms);
}
