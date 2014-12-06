#include "anime.h"

Anime::Anime(QObject *parent) : QObject(parent) {}

void Anime::finishReload() { emit finishedReloading(); needLoad = false;}
bool Anime::needsLoad() { return needLoad; }


QString     Anime::getID()                                { return id; }
void        Anime::setID(QString id)                      { this ->id = id; }

QStringList Anime::getSynonyms()                          { return synonyms; }
void        Anime::addSynonym(QString synonym)            { synonyms.append(synonym); }

QString     Anime::getRomajiTitle()                       { return romaji_title; }
void        Anime::setRomajiTitle(QString title)          { this ->romaji_title = title; }

QString     Anime::getJapaneseTitle()                     { return japanese_title; }
void        Anime::setJapaneseTitle(QString title)        { this ->japanese_title = title; }

QString     Anime::getEnglishTitle()                      { return english_title; }
void        Anime::setEnglishTitle(QString title)         { this ->english_title = title; }

QString     Anime::getType()                              { return type; }
void        Anime::setType(QString type)                  { this ->type = type; }

QString     Anime::getAiringStatus()                      { return airing_status; }
void        Anime::setAiringStatus(QString airing_status) { this ->airing_status = airing_status; }

int         Anime::getEpisodeCount()                      { return episode_count; }
void        Anime::setEpisodeCount(int episode_count)     { this ->episode_count = episode_count; }

QString     Anime::getAverageScore()                      { return average_score; }
void        Anime::setAverageScore(QString average_score) { this ->average_score = average_score; }
