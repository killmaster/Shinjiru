#include "animepanel.h"
#include "ui_animepanel.h"
#include "mainwindow.h"

#include <QDebug>
#include <QJsonDocument>
#include <limits>

AnimePanel::AnimePanel(QWidget *parent, Anime *anime, int score_type) : QDialog(parent), ui(new Ui::AnimePanel) {
  qDebug() << "Loading anime panel for anime" << anime->getTitle();
  ui->setupUi(this);
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

  this->anime = anime;
  this->score_type = score_type;
  QString airing_status = anime->getAiringStatus();
  airing_status = airing_status.at(0).toUpper() + airing_status.right(airing_status.length() - 1);

  QString score = anime->getMyScore();

  if(score_type == 0 || score_type == 1) {
    QSpinBox *score_container = new QSpinBox(this);

    score_container->setMaximum(score_type == 0 ? 10 : score_type == 1 ? 100 : 5);
    score_container->setMinimum(score_type == 2 ? 1 : 0);
    score_container->setValue(score.split(" ").at(0).toInt());

    ui->myDetailForm->setWidget(2, QFormLayout::FieldRole, score_container);
    score_widget = score_container;

  } else if (score_type == 2) {
    QComboBox *score_container = new QComboBox(this);

    score_container->addItem("0 ★");
    score_container->addItem("1 ★");
    score_container->addItem("2 ★");
    score_container->addItem("3 ★");
    score_container->addItem("4 ★");
    score_container->addItem("5 ★");

    score_container->setCurrentText(score);

    ui->myDetailForm->setWidget(2, QFormLayout::FieldRole, score_container);
    score_widget = score_container;
  } else if (score_type == 3) {
    QComboBox *score_container = new QComboBox(this);

    score_container->addItem("");
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

  ui->lblTitle->setText(anime->getTitle());
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

  QString my_status = anime->getMyStatus();

  my_status = my_status.at(0).toUpper() + my_status.right(my_status.length() - 1);
  if(my_status == "Plan to watch") my_status = "Plan to Watch";
  ui->comboStatus->setCurrentText(my_status);



  if(anime->needsLoad() || anime->needsCover()) {
    connect(anime, SIGNAL(finishedReloading()), SLOT(refreshDisplay()));
    connect(anime, SIGNAL(new_image()), SLOT(repaint()));

    User::sharedUser()->loadAnimeData(anime, true);
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
  QMargins m = ui->scrollLayout->contentsMargins();
  m.setRight(20 + cover.width());
  ui->scrollLayout->setContentsMargins(m);

  p.drawPixmap(width() - 235, 10, cover.width(), cover.height(), cover);
  p.drawRect  (width() - 235, 10, cover.width(), cover.height());

  event->accept();
}

void AnimePanel::refreshDisplay() {
  QStringList synonyms = anime->getSynonyms();

  if(anime->getTitle() != anime->getEnglishTitle()) {
    if(!synonyms.contains(anime->getEnglishTitle()))
      synonyms.append(anime->getEnglishTitle());
  }

  if(anime->getTitle() != anime->getRomajiTitle()) {
    if(!synonyms.contains(anime->getRomajiTitle()))
      synonyms.append(anime->getRomajiTitle());
  }

  if(anime->getTitle() != anime->getJapaneseTitle()) {
    if(!synonyms.contains(anime->getJapaneseTitle()))
      synonyms.append(anime->getJapaneseTitle());
  }

  ui->lblSynonyms->setText(synonyms.join(", "));
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

  if(score_type == 0 || score_type == 1) {
    i_score = static_cast<QSpinBox *>(score_widget)->value();
    score = QString::number(i_score);
  } else if (score_type == 3 || score_type == 2) {
    score = static_cast<QComboBox *>(score_widget)->currentText();
  } else {
    d_score = static_cast<QDoubleSpinBox *>(score_widget)->value();
    score = QString::number(d_score);
  }

  if(anime->getMyNotes() != notes || anime->getMyProgress() != eps || anime->getMyScore() != score || anime->getMyStatus() != status || anime->getMyRewatch() != rewatch) {
    QMap<QString, QString> data;
    data.insert("id",                 anime->getID());

    if(anime->getMyStatus() != status.toLower()) {
      data.insert("list_status",      status.toLower());
      anime->setMyStatus(status.toLower());
    }

    if(anime->getMyScore() != score) {
      data.insert("score",            score);
      anime->setMyScore(score);
    }

    if(anime->getMyProgress() != eps) {
      data.insert("episodes_watched", QString::number(eps));
      anime->setMyProgress(eps);
    }

    if(anime->getMyRewatch() != rewatch) {
      data.insert("rewatched",        QString::number(rewatch));
      anime->setMyRewatch(rewatch);
    }

    if(anime->getMyNotes() != notes) {
      data.insert("notes",            notes);
      anime->setMyNotes(notes);
    }

    API::sharedAPI()->sharedAniListAPI()->put(API::sharedAPI()->sharedAniListAPI()->API_EDIT_LIST, data);
  }

  done(QDialog::Accepted);
}
