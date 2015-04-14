/* Copyright 2015 Kazakuri */

#include "./torrentrss.h"
#include "../settings.h"

TorrentRSS::TorrentRSS(QWidget *parent) : QWidget(parent), currentReply(0) {
  connect(&manager, SIGNAL(finished(QNetworkReply*)),
          SLOT(finished(QNetworkReply*)));
  links = new QList<QString>;
  titles = new QList<QString>;
}

TorrentRSS::~TorrentRSS() {
  delete links;
  delete titles;
}

void TorrentRSS::get(const QUrl &url) {
  QNetworkRequest request(url);
  if (currentReply) {
    currentReply->disconnect(this);
    currentReply->deleteLater();
  }
  currentReply = manager.get(request);
  connect(currentReply, SIGNAL(readyRead()), SLOT(readyRead()));
  connect(currentReply, SIGNAL(metaDataChanged()), SLOT(metaDataChanged()));
  connect(currentReply, SIGNAL(error(QNetworkReply::NetworkError)),
          SLOT(error(QNetworkReply::NetworkError)));
}

void TorrentRSS::fetch() {
  xml.clear();
  Settings *s = new Settings;
  // QUrl url(s->getValue(Settings::TorrentRSSURL,
  // "http://tokyotosho.info/rss.php?filter=1&zwnj=0").toString());
  QUrl url("http://tokyotosho.info/rss.php?filter=1&zwnj=0");
  get(url);

  delete s;
}

void TorrentRSS::metaDataChanged() {
  QUrl redirectionTarget = currentReply->
          attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
  if (redirectionTarget.isValid()) {
    get(redirectionTarget);
  }
}

void TorrentRSS::readyRead() {
  int statusCode = currentReply->
          attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
  if (statusCode >= 200 && statusCode < 300) {
    QByteArray data = currentReply->readAll();
    xml.addData(data);
    parseXml();
  }
}

void TorrentRSS::finished(QNetworkReply *reply) {
  Q_UNUSED(reply);
  emit done();
}

void TorrentRSS::parseXml() {
  bool metItem = false;
  while (!xml.atEnd()) {
    xml.readNext();
    if (xml.isStartElement()) {
      if (xml.name() == "item") {
        metItem = true;
      }
      currentTag = xml.name().toString();
    } else if (xml.isEndElement()) {
      if (xml.name() == "item") {
        titles->append(titleString);
        links->append(linkString);
        titleString.clear();
        linkString.clear();
        metItem = false;
      }
      currentTag = "";
    } else if (xml.isCharacters() && !xml.isWhitespace() && metItem) {
      if (currentTag == "title") {
        titleString += xml.text().toString();
      } else if (currentTag == "description") {
        QString description = xml.text().toString();
        int magnetStart = description.indexOf("magnet:");
        int length = description.indexOf(">Magnet") - magnetStart - 1;
        linkString += description.mid(magnetStart, length);
      }
    }
  }
}

QList<QString>* TorrentRSS::getTitles() {
  return titles;
}

QList<QString>* TorrentRSS::getLinks() {
  return links;
}

void TorrentRSS::error(QNetworkReply::NetworkError) {
  currentReply->disconnect(this);
  currentReply->deleteLater();
  currentReply = 0;
}
