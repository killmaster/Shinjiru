#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "browseanime.h"

QUrl MainWindow::addPage(QUrl url, int page) {
  QString url_s = url.toDisplayString();

  if(url.hasQuery()) {
    if(url_s.contains("page=[0-9]")) {
      url_s.replace("page=[0-9]", "page=" + QString::number(page));
    } else
      url_s += "&page=" + QString::number(page);
  } else {
    url_s += "?page=" + QString::number(page);
  }

  return QUrl(url_s);
}

QUrl MainWindow::addQuery(QUrl url, QString key, QString value) {
  QString url_s = url.toDisplayString();

  if(url.hasQuery()) {
    url_s += "&" + key + "=" + value;
  } else {
    url_s += "?" + key + "=" + value;
  }

  return QUrl(url_s);
}

void MainWindow::loadBrowserData() {
  QUrl url = API::sharedAPI()->sharedAniListAPI()->API_BROWSE;

  QString season = ui->comboSeason->currentText();
  QString year   = ui->comboYear->currentText();
  QString type   = ui->comboType->currentText();
  QString status = ui->comboStatus->currentText();

  // Clear the browser view
  QLayoutItem *item;

  while((item = layout2->takeAt(0))){
    delete item->widget();
    delete item;
  }

  if(!season.isEmpty()) {
    url = addQuery(url, "season", season);
  }

  if(!year.isEmpty()) {
    url = addQuery(url, "year", year);
  }

  if(!type.isEmpty()) {
    url = addQuery(url, "type", type);
  }

  if(!status.isEmpty()) {
    url = addQuery(url, "status", status);
  }

  QStringList genres;
  QStringList exclude;

  for(int i = 0; i < ui->genreList->count(); i++) {
    QCheckBox *w = static_cast<QCheckBox *>(dynamic_cast<QWidgetItem *>(ui->genreList->itemAt(i))->widget());

    if(w->checkState() == Qt::PartiallyChecked) {
      exclude.append(w->text());
    } else if(w->checkState() == Qt::Checked) {
      genres.append(w->text());
    }
  }

  if(!genres.isEmpty()) {
    url = addQuery(url, "genres", genres.join(","));
  }

  if(!exclude.isEmpty()) {
    url = addQuery(url, "genres_exclude", exclude.join(","));
  }

  // Load the results for the requested type
  QJsonArray browse_results = API::sharedAPI()->sharedAniListAPI()->get(url).array();

  for(int i = 0; i <= browse_results.size(); i++) {
    QJsonObject anime = browse_results.at(i).toObject();

    Anime *a = User::sharedUser()->getAnimeByTitle(anime.value("title_romaji").toString());

    if(a == 0) {
      a = new Anime();
      a->setID(QString::number(anime.value("id").toInt()));
      a->setMyProgress(0);
      a->setMyNotes("");
      a->setMyRewatch(0);
      a->setMyStatus("");

      if(a->getID() == "0") {
        delete a;
        continue;
      }
    }

    BrowseAnime *s = new BrowseAnime(this, User::sharedUser()->scoreType());

    if(a->needsLoad() || a->needsCover()) {
      User::sharedUser()->loadAnimeData(a, true);

      QEventLoop evt;
      connect(a, SIGNAL(finishedReloading()), &evt, SLOT(quit()));
      evt.exec();
    }

    s->setAnime(a);

    layout2->addWidget(s);

    // Do we need to keep loading?
    if(season != ui->comboSeason->currentText() ||
       year != ui->comboYear->currentText() ||
       type != ui->comboType->currentText() ||
       status != ui->comboStatus->currentText()) {
      return;
    }

    int width = layout2->geometry().width();
    int cwidth = layout2->contentsWidth();

    if(cwidth < 0) {
      width = this->width() - 2;
      cwidth = this->width() - (this->width() % 200);
    }

    layout2->setContentsMargins((width-cwidth)/2, 0, 0, 0);
  }
}
