#ifndef TORRENTS_H
#define TORRENTS_H

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QWidget>
#include <QBuffer>
#include <QXmlStreamReader>
#include <QUrl>
#include <QList>

class Torrents : public QWidget
{
  Q_OBJECT

public:
  Torrents(QWidget *widget = 0);

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
