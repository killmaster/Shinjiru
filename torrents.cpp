#include "torrents.h"

Torrents::Torrents(QWidget *parent) : QWidget(parent), currentReply(0) {
  connect(&manager, SIGNAL(finished(QNetworkReply*)),
          SLOT(finished(QNetworkReply*)));
  links = new QList<QString>;
  titles = new QList<QString>;
}

void Torrents::get(const QUrl &url) {
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

void Torrents::fetch() {
  xml.clear();
  QUrl url("http://tokyotosho.info/rss.php?filter=1&zwnj=0");
  get(url);
}

void Torrents::metaDataChanged() {
  QUrl redirectionTarget = currentReply->
          attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
  if (redirectionTarget.isValid()) {
    get(redirectionTarget);
  }
}

void Torrents::readyRead() {
  int statusCode = currentReply->
          attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
  if (statusCode >= 200 && statusCode < 300) {
    QByteArray data = currentReply->readAll();
    xml.addData(data);
    parseXml();
  }
}

void Torrents::finished(QNetworkReply *reply) {
  Q_UNUSED(reply);
  emit done();
}

void Torrents::parseXml() {
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
      if (currentTag == "title")
        titleString += xml.text().toString();
      else if (currentTag == "description") {
        QString description = xml.text().toString();
        int magnetStart = description.indexOf("magnet:");
        int length= description.indexOf(">Magnet") - magnetStart - 1;
        linkString += description.mid(magnetStart, length);
      }
    }
  }
}

QList<QString>* Torrents::getTitles() {
  return titles;
}

QList<QString>* Torrents::getLinks() {
  return links;
}



void Torrents::error(QNetworkReply::NetworkError) {
  currentReply->disconnect(this);
  currentReply->deleteLater();
  currentReply = 0;
}
