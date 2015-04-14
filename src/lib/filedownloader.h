/* Copyright 2015 Kazakuri */

#ifndef SRC_LIB_FILEDOWNLOADER_H_
#define SRC_LIB_FILEDOWNLOADER_H_

#include <QObject>
#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

class FileDownloader : public QObject {
  Q_OBJECT

 public:
  explicit FileDownloader(QUrl imageUrl, QObject *parent = 0);
  virtual ~FileDownloader();
  QByteArray downloadedData() const;

 signals:
  void downloaded();

 private slots:  // NOLINT
  void fileDownloaded(QNetworkReply* pReply);

 private:
  QNetworkAccessManager m_WebCtrl;
  QByteArray m_DownloadedData;
};

#endif  // SRC_LIB_FILEDOWNLOADER_H_
