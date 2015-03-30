#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "browseanime.h"

void MainWindow::loadSeasonBrowser() {
  ui->browseTabs->setCurrentIndex(0);
}

void MainWindow::loadAiringBrowser() {
  ui->browseTabs->setCurrentIndex(1);
}

void MainWindow::loadUpcomingBrowser() {
  ui->browseTabs->setCurrentIndex(2);
}

void MainWindow::loadRecentBrowser() {
  ui->browseTabs->setCurrentIndex(3);
}

QUrl MainWindow::addPage(QUrl url, int page) {
  QString url_s = url.toDisplayString();

  qDebug() << url_s;

  if(url.hasQuery()) {
    if(url_s.contains("page=[0-9]")) {
      url_s.replace("page=[0-9]", "page=" + QString::number(page));
    } else
      url_s += "&page=" + QString::number(page);
  } else {
    url_s += "?page=" + QString::number(page);
  }

  qDebug() << url_s;

  return QUrl(url_s);
}

void MainWindow::loadBrowserData() {
  // Get browser data
  QString type = "";
  int ci = ui->browseTabs->currentIndex();
  switch(ui->browseTabs->currentIndex()) {
    case 0:
      type = "season";
      break;
    case 1:
      type = "airing";
      break;
    case 2:
      type = "upcoming";
      break;
    case 3:
      type = "recent";
      break;
  }

  QUrl url;

  QString season;
  QString year;

  if(type != "season") {
    url = API::sharedAPI()->sharedAniListAPI()->API_BROWSE(type);
  } else {
    season = ui->comboSeason->currentText();
    year = ui->comboYear->currentText();

    url = API::sharedAPI()->sharedAniListAPI()->API_SEASON(season, year);
  }

  // Clear the browser view
  QLayoutItem *item;

  while((item = layout2->takeAt(0))){
    delete item->widget();
    delete item;
  }

  // Load the results for the requested type
  QJsonObject browse_results = API::sharedAPI()->sharedAniListAPI()->get(url).object();

  for(int i = 1; i <= browse_results.value("last_page").toInt(1); i++) {
    if(i != 1) {
      browse_results = API::sharedAPI()->sharedAniListAPI()->get(addPage(url, i)).object();
    }

    for(QJsonValue val : browse_results.value("data").toArray()) {
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

      // Do we need to keep loading?
      if(ui->browseTabs->currentIndex() != ci ||
        (ci == 0 && season != ui->comboSeason->currentText() && year != ui->comboYear->currentText())) {
        return;
      }

      BrowseAnime *s = new BrowseAnime(this, User::sharedUser()->scoreType());
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
