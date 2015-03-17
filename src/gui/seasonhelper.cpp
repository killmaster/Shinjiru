#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "seasonanime.h"

void MainWindow::loadSelectedSeason() {
  QString season = ui->comboSeason->currentText();
  QString year = ui->comboYear->currentText();

  QString current_season = ui->comboSeason->currentText() + " " + ui->comboYear->currentText();

  QLayoutItem *item;

  while(item = layout2->takeAt(0)){
    delete item->widget();
    delete item;
  }

  QJsonObject season_results = API::sharedAPI()->sharedAniListAPI()->get(API::sharedAPI()->sharedAniListAPI()->API_SEASON(season, year)).object();

  for(int i = 1; i <= season_results.value("last_page").toInt(1); i++) {
    if(i != 1) {
      season_results = API::sharedAPI()->sharedAniListAPI()->get(API::sharedAPI()->sharedAniListAPI()->API_SEASON(season, year, QString::number(i))).object();
    }

    for(QJsonValue val : season_results.value("data").toArray()) {
      QJsonObject anime = val.toObject();

      Anime *a = User::sharedUser()->getAnimeByTitle(anime.value("title_romaji").toString());

      if(a == 0) {
        a = new Anime();
        a->setID(QString::number(anime.value("id").toInt()));
        a->setMyProgress(0);
        a->setMyNotes("");
        a->setMyRewatch(0);
      }

      if(a->needsLoad() || a->needsCover()) {
        User::sharedUser()->loadAnimeData(a, true);

        QEventLoop evt;
        connect(a, SIGNAL(finishedReloading()), &evt, SLOT(quit()));
        evt.exec();
      }

      QString current_season_text = ui->comboSeason->currentText() + " " + ui->comboYear->currentText();

      if(current_season_text != current_season) {
        return;
      }

      SeasonAnime *s = new SeasonAnime(this, User::sharedUser()->scoreType());
      s->setAnime(a);

      layout2->addWidget(s);

      int width = layout2->geometry().width();
      int cwidth = layout2->contentsWidth();

      if(cwidth < 0) {
        width = this->width() - 2;
        cwidth = this->width() - (this->width() % 200);
      }

      layout2->setContentsMargins((width-cwidth)/2, 0, 0, 0);
    }
  }
}
