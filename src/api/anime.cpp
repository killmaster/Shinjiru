#include "anime.h"

Anime::Anime(QObject *parent) : QObject(parent) {
  anime_image_control = nullptr;
  this->needCover = true;
  this->needLoad = true;
}

void Anime::finishReload() { emit finishedReloading(); needLoad = false;}
bool Anime::needsLoad()    { return needLoad; }
bool Anime::needsCover()   { return needCover; }


QString     Anime::getID()                                { return id; }
void        Anime::setID(QString id)                      { this ->id = id.trimmed(); }

QStringList Anime::getSynonyms()                          { return synonyms; }
void        Anime::addSynonym(QString synonym)            { if(!synonyms.contains(synonym)) synonyms.append(synonym); }

QString     Anime::getRomajiTitle()                       { return romaji_title; }
void        Anime::setRomajiTitle(QString title)          { this ->romaji_title = title; }

QString     Anime::getJapaneseTitle()                     { return japanese_title; }
void        Anime::setJapaneseTitle(QString title)        { this ->japanese_title = title; }

QString     Anime::getEnglishTitle()                      { return english_title; }
void        Anime::setEnglishTitle(QString title)         { this ->english_title = title; }

QString     Anime::getTitle()                             { return title; }
void        Anime::setTitle(QString title)                { this ->title = title; }

QString     Anime::getType()                              { return type; }
void        Anime::setType(QString type)                  { this ->type = type; }

QString     Anime::getAiringStatus()                      { return airing_status; }
void        Anime::setAiringStatus(QString airing_status) { this ->airing_status = airing_status; }

int         Anime::getEpisodeCount()                      { return episode_count; }
void        Anime::setEpisodeCount(int episode_count)     { this ->episode_count = episode_count; }

QString     Anime::getAverageScore()                      { return average_score; }
void        Anime::setAverageScore(QString average_score) { this ->average_score = average_score; }

QString     Anime::getSynopsis()                          { return synopsis; }
void        Anime::setSynopsis(QString synopsis)          { this ->synopsis = synopsis; }

QUrl        Anime::getCoverURL()                          { return cover_url; }
void        Anime::setCoverURL(QUrl cover_url)            { this ->cover_url = cover_url; }

QPixmap     Anime::getCoverImage()                        { return cover_image; }
void        Anime::setCoverImageData(QByteArray data)     { this->cover_image.loadFromData(data);
                                                            emit new_image(); }

int         Anime::getMyProgress()                        { return my_progress; }
void        Anime::setMyProgress(int progress)            { this ->my_progress = progress; }

QString     Anime::getMyScore()                           { return my_score; }
void        Anime::setMyScore(QString score)              { this ->my_score = score; }

QString     Anime::getMyNotes()                           { return my_notes; }
void        Anime::setMyNotes(QString notes)              { this ->my_notes = notes; }

int         Anime::getMyRewatch()                         { return rewatch_count; }
void        Anime::setMyRewatch(int rewatch)              { this ->rewatch_count = rewatch; }

QString     Anime::getMyStatus()                          { return my_status; }
void        Anime::setMyStatus(QString status)            { this ->my_status = status; }

int         Anime::getDuration()                          { return duration; }
void        Anime::setDuration(int duration)              { this ->duration = duration; }

int         Anime::getCountdown()                         { return countdown; }
void        Anime::setCountdown(int c)                    { this ->countdown = c; }

int         Anime::getNextEpisode()                       { return next_episode; }
void        Anime::setNextEpisode(int next)               { this ->next_episode = next; }

void Anime::downloadCover() {
  this->needCover = false;
  anime_image_control = new FileDownloader(getCoverURL());
  connect(anime_image_control, &FileDownloader::downloaded, [&]() {
    this->setCoverImageData(anime_image_control->downloadedData());
    anime_image_control->deleteLater();
  });
}
