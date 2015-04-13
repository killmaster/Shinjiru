/* Copyright 2015 Kazakuri */

#ifndef SRC_API_ANIME_H_
#define SRC_API_ANIME_H_

#include <QString>
#include <QStringList>
#include <QUrl>
#include <QPixmap>

#include "../lib/filedownloader.h"
#include "./api.h"

class Anime : public QObject {
  Q_OBJECT

 public:
  explicit Anime(QObject *parent = 0);

  void finishReload();
  bool needsLoad();
  bool needsCover();

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

  QString getTitle();
  void setTitle(QString title);

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

  int getDuration();
  void setDuration(int duration);

  int getCountdown();
  void setCountdown(int c);

  int getNextEpisode();
  void setNextEpisode(int next);

  bool isDefaultHidden();
  void setDefaultHidden(bool hidden);

  QList<int> getCustomLists();
  void setCustomLists(QList<int> lists);

  bool hasAiringSchedule();
  void setAiringSchedule(bool schedule);

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
  QString title;
  QString type;
  QString airing_status;
  int episode_count;
  QString average_score;
  bool needLoad = true;
  bool needCover = true;
  int duration;
  QString synopsis;
  QUrl cover_url;
  QPixmap cover_image;
  bool has_schedule;

  int countdown;
  int next_episode;

  bool default_hidden;
  QList<int> custom_lists;

  FileDownloader *anime_image_control;

  int my_progress;
  QString my_score;
  QString my_notes;
  int rewatch_count;
  QString my_status;
};

#endif  // SRC_API_ANIME_H_
