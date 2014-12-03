#ifndef ANIME_H
#define ANIME_H

#include <QString>
#include <QStringList>

class Anime {
public:
  Anime();

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

private:
  QString id;
  QStringList synonyms;
  QString romaji_title;
  QString japanese_title;
  QString english_title;
  QString type;
  QString airing_status;
  int episode_count;
};

#endif // ANIME_H
