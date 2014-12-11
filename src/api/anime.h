#ifndef ANIME_H
#define ANIME_H

#include <QString>
#include <QStringList>
#include <QUrl>
#include <QPixmap>

#include "../lib/filedownloader.h"

class Anime : public QObject {
  Q_OBJECT

public:
  Anime(QObject *parent = 0);

  void finishReload();
  bool needsLoad();

  QString getID();
  void setID(QString id);

  QStringList getSynonyms();
  void addSynonym(QString synonym);

  QString getRomajiTitle();
  void setRomajiTitle(QString title);

  QString getJapaneseTitle();
  void setJapaneseTitle(QString title);

  QString getEnglishTitle();
  void setEnglishTitle(QString title);

  QString getType();
  void setType(QString type);

  QString getAiringStatus();
  void setAiringStatus(QString airing_status);

  int getEpisodeCount();
  void setEpisodeCount(int episode_count);

  QString getAverageScore();
  void setAverageScore(QString average_score);

  QString getSynopsis();
  void setSynopsis(QString synopsis);

  QUrl getCoverURL();
  void setCoverURL(QUrl cover_url);

  QPixmap getCoverImage();
  void setCoverImageData(QByteArray data);

  int getMyProgress();
  void setMyProgress(int progress);

  QString getMyScore();
  void setMyScore(QString score);

  QString getMyNotes();
  void setMyNotes(QString notes);

  int getMyRewatch();
  void setMyRewatch(int rewatch);

  QString getMyStatus();
  void setMyStatus(QString status);

  void downloadCover();

signals:
  void finishedReloading();
  void new_image();

private:
  QString id;
  QStringList synonyms;
  QString romaji_title;
  QString japanese_title;
  QString english_title;
  QString type;
  QString airing_status;
  int episode_count;
  QString average_score;
  bool needLoad = true;
  QString synopsis;
  QUrl cover_url;
  QPixmap cover_image;

  FileDownloader *anime_image_control;

  int my_progress;
  QString my_score;
  QString my_notes;
  int rewatch_count;
  QString my_status;
};

#endif // ANIME_H