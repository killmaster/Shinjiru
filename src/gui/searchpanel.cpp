#include "searchpanel.h"
#include "ui_searchpanel.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>
#include <QMessageBox>

#include "../api/api.h"
#include "../api/user.h"
#include "animepanel.h"
#include "../api/anime.h"

const QString RESULT_TEMPLATE =
"<a href=\"http://localhost/%href%\"> \
  <div class=\"result\"> \
    <img src=\"%url%\" /> \
    %title% \
  </div> \
</a>";

const QString KEYWORD_REPLACE = "<!-- SEARCH RESULTS -->";

SearchPanel::SearchPanel(QWidget *parent) : QDialog(parent), ui(new Ui::SearchPanel) {
  qDebug() << "Launching search panel";
  ui->setupUi(this);
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

  connect(ui->searchButton, &QPushButton::clicked, [&]() {
    QUrl search_url = API::sharedAPI()->sharedAniListAPI()->API_ANIME_SEARCH(ui->lineEdit->text());
    QJsonArray results = API::sharedAPI()->sharedAniListAPI()->get(search_url).array();

    QFile h(":/search_results.html");
    h.open(QFile::ReadOnly);

    QString html = h.readAll();

    for(QJsonValue v : results) {
      QJsonObject anime = v.toObject();
      QString title = anime.value("title_romaji").toString();
      QString image_url = anime.value("image_url_med").toString();
      QString id = QString::number(anime.value("id").toInt());

      QString replace_str = RESULT_TEMPLATE;
      replace_str.replace("%url%", image_url);
      replace_str.replace("%title%", title);
      replace_str.replace("%href%", id + "#" + title);

      html.replace(KEYWORD_REPLACE, replace_str + KEYWORD_REPLACE);
    }

    ui->webView->setHtml(html);
    ui->webView->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
  });

  connect(ui->webView, &QWebView::linkClicked, [&](const QUrl &url) {
    QString url_s = url.toDisplayString().replace("http://localhost/", "");

    QStringList comp = url_s.split("#");

    QString id = comp.first();
    comp.removeFirst();
    QString title = comp.join("#");

    Anime *a = User::sharedUser()->getAnimeByTitle(title);

    if(a == 0) {
      a = new Anime(0);
      a->setID(id);
      a->setMyProgress(0);
      a->setMyNotes("");
      a->setMyRewatch(0);
    }

    QEventLoop evt;
    User::sharedUser()->loadAnimeData(a, false);
    connect(a, SIGNAL(finishedReloading()), &evt, SLOT(quit()));
    evt.exec();

    QString old_status = a->getMyStatus();

    AnimePanel *ap = new AnimePanel(0, a, User::sharedUser()->scoreType());
    ap->show();

    connect(ap, &AnimePanel::accepted, this, [&, old_status]() {
      if(a->getMyStatus() != old_status) {
        User::sharedUser()->removeFromList(old_status, a);
        User::sharedUser()->addToList(a->getMyStatus(), a);
      }

      User::sharedUser()->animeChanged();
    });
  });
}

SearchPanel::~SearchPanel() {
  delete ui;
}

void SearchPanel::setSearch(QString text) {
  ui->lineEdit->setText(text);
  ui->searchButton->click();
}
