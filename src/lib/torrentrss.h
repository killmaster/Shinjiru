#ifndef TORRENT_RSS_H
#define TORRENT_RSS_H

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QWidget>
#include <QBuffer>
#include <QXmlStreamReader>
#include <QUrl>
#include <QList>

class TorrentRSS : public QWidget
{
  Q_OBJECT

public:
  TorrentRSS(QWidget *widget = 0);

  QList<QString> *getLinks();
  QList<QString> *getTitles();

public slots:
  void fetch();
  void finished(QNetworkReply *reply);
  void readyRead();
  void metaDataChanged();
  void error(QNetworkReply::NetworkError);

signals:
  void done();

private:
  void parseXml();
  void get(const QUrl &url);

  QXmlStreamReader xml;
  QString currentTag;
  QString linkString;
  QString titleString;

  QList<QString> *titles;
  QList<QString> *links;

  QNetworkAccessManager manager;
  QNetworkReply *currentReply;
};

#endif
