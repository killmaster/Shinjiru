#include "animepanel.h"
#include "ui_animepanel.h"
#include "mainwindow.h"

#include <QDebug>
#include <limits>

AnimePanel::AnimePanel(QWidget *parent, Anime *anime, int score_type, AniListAPI *api) : QDialog(parent), ui(new Ui::AnimePanel) {
  ui->setupUi(this);
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

  this->anime = anime;
  this->score_type = score_type;
  this->api = api;
  QString airing_status = anime->getAiringStatus();
  airing_status = airing_status.at(0).toUpper() + airing_status.right(airing_status.length() - 1);

  QString score = anime->getMyScore();

  if(score_type == 0 || score_type == 1 || score_type == 2) {
    QSpinBox *score_container = new QSpinBox(this);

    score_container->setMaximum(score_type == 0 ? 10 : score_type == 1 ? 100 : 5);
    score_container->setMinimum(score_type == 2 ? 1 : 0);
    score_container->setValue(score.split(" ").at(0).toInt());

    ui->myDetailForm->setWidget(2, QFormLayout::FieldRole, score_container);
    score_widget = score_container;

  } else if (score_type == 3) {
    QComboBox *score_container = new QComboBox(this);

    score_container->addItem(":(");
    score_container->addItem(":|");
    score_container->addItem(":)");

    score_container->setCurrentText(score);

    ui->myDetailForm->setWidget(2, QFormLayout::FieldRole, score_container);
    score_widget = score_container;
  } else {
    QDoubleSpinBox *score_container = new QDoubleSpinBox(this);

    score_container->setMaximum(10.0);
    score_container->setMinimum(0);
    score_container->setDecimals(1);
    score_container->setSingleStep(0.1);
    score_container->setValue(score.toDouble());

    ui->myDetailForm->setWidget(2, QFormLayout::FieldRole, score_container);
    score_widget = score_container;
  }

  ui->lblTitle->setText(anime->getRomajiTitle());
  ui->txtType->setText(anime->getType());
  ui->txtEpisodes->setText(QString::number(anime->getEpisodeCount()));
  ui->txtScore->setText(anime->getAverageScore());
  ui->txtAiring->setText(airing_status);

  int max_ep = anime->getEpisodeCount();
  if(max_ep == 0) max_ep = std::numeric_limits<int>::max();

  ui->spinEps->setMaximum(max_ep);
  ui->spinEps->setValue(anime->getMyProgress());
  ui->txtNotes->setText(anime->getMyNotes());
  ui->spinRewatch->setValue(anime->getMyRewatch());
  ui->comboStatus->setCurrentText(anime->getMyStatus());

  if(anime->needsLoad()) {
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

void AnimePanel::accept() {
  QString score = "";
  int i_score = 0;
  double d_score = 0.0;
  QString notes = ui->txtNotes->text();
  int eps = ui->spinEps->value();
  QString status = ui->comboStatus->currentText();
  int rewatch = ui->spinRewatch->value();

  if(score_type == 0 || score_type == 1 || score_type == 2) {
    i_score = static_cast<QSpinBox *>(score_widget)->value();
    score = QString::number(i_score);

    if(score_type == 2) score += " Star";

  } else if (score_type == 3) {
    score = static_cast<QComboBox *>(score_widget)->currentText();
  } else {
    d_score = static_cast<QDoubleSpinBox *>(score_widget)->value();
    score = QString::number(d_score);
  }

  if(anime->getMyNotes() != notes || anime->getMyProgress() != eps || anime->getMyScore() != score
     || anime->getMyStatus() != status || anime->getMyRewatch() != rewatch) {
    QMap<QString, QString> data;
    data.insert("id",               anime->getID());
    data.insert("list_status",      status.toLower());
    data.insert("score",            score);
    data.insert("episodes_watched", QString::number(eps));
    data.insert("rewatched",        QString::number(rewatch));
    data.insert("notes",            notes);

    api->put(api->API_EDIT_LIST, data);
  }

  done(QDialog::Accepted);
}
