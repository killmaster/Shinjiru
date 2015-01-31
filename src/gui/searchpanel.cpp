#include "searchpanel.h"
#include "ui_searchpanel.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>

#include "../api/api.h"
#include "../api/user.h"

const QString RESULT_TEMPLATE =
"<a href=\"\"> \
  <div class=\"result\"> \
    <img src=\"%url%\" /> \
    %title% \
  </div> \
</a>";

const QString KEYWORD_REPLACE = "<!-- SEARCH RESULTS -->";

SearchPanel::SearchPanel(QWidget *parent) : QDialog(parent), ui(new Ui::SearchPanel) {
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

      QString replace_str = RESULT_TEMPLATE;
      replace_str.replace("%url%", image_url);
      replace_str.replace("%title%", title);

      html.replace(KEYWORD_REPLACE, replace_str + KEYWORD_REPLACE);
    }

    ui->webView->setHtml(html);
  });
}

SearchPanel::~SearchPanel() {
  delete ui;
}
