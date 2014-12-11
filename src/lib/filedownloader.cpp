#include "filedownloader.h"

#include <QDebug>

FileDownloader::FileDownloader(QUrl imageUrl, QObject *parent) : QObject(parent) {
  connect(&m_WebCtrl, SIGNAL(finished(QNetworkReply*)), this, SLOT(fileDownloaded(QNetworkReply*)));
  QNetworkRequest request(imageUrl);

  m_WebCtrl.get(request);
}

FileDownloader::~FileDownloader() {

}

void FileDownloader::fileDownloaded(QNetworkReply* pReply) {
  if(pReply->error() != QNetworkReply::NoError)
    qDebug() << pReply->errorString();

  m_DownloadedData = pReply->readAll();
  pReply->deleteLater();
  emit downloaded();
}

QByteArray FileDownloader::downloadedData() const {
  return m_DownloadedData;
}