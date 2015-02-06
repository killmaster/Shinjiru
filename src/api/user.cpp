#include "user.h"
#include "anime.h"

#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QEventLoop>

User* User::m_Instance = 0;

User* User::sharedUser() {
  static QMutex mutex;
  if(! m_Instance) {
    mutex.lock();

    if(!m_Instance) {
      m_Instance = new User;
    }

    mutex.unlock();
  }

  return m_Instance;
}

User::User() : QObject(0) {
  qDebug() << "Requesting current user from AniList...";
  QJsonObject result = API::sharedAPI()->sharedAniListAPI()->get(API::sharedAPI()->sharedAniListAPI()->API_USER).object();

  user_image_control = nullptr;

  this->setDisplayName      (result.value("display_name")     .toString());
  this->setScoreType        (result.value("score_type")       .toInt());
  this->setProfileImageURL  (result.value("image_url_med")    .toString());
  this->setTitleLanguage    (result.value("title_language")   .toString());
  this->setAnimeTime        (result.value("anime_time")       .toInt());
  this->setCustomLists      (result.value("custom_list_anime").toArray().toVariantList());
  this->setNotificationCount(result.value("notifications")    .toInt());

  this->loadProfileImage();
}

User::~User() {
  if(user_image_control != nullptr) delete user_image_control;
  for(Anime *a : anime_list){
    delete a;
  }
}

bool User::loadProfileImage() {
  if(profileImageURL().isEmpty()) return false;

  QUrl url(profileImageURL());

  qDebug() << "Downloading user image" << profileImageURL() << "for" << displayName();

  user_image_control = new FileDownloader(url, this);

  QEventLoop event;
  connect(user_image_control, SIGNAL(downloaded()), &event, SLOT(quit()));
  event.exec();

  this->setUserImage(user_image_control->downloadedData());

  return true;
}

void User::setScoreType(const int &score_type) {
  this->score_type = score_type;

  switch(score_type) {
    case 0:
      this->max_score = "10";   break;
    case 1:
      this->max_score = "100";  break;
    case 2:
      this->max_score = "5";    break;
    case 3:
      this->max_score = "3";    break;
    case 4:
      this->max_score = "10.0"; break;
  };
}

void User::loadUserList() {
  qDebug() << "Fetching user list";

  QJsonObject user_list_data = API::sharedAPI()->sharedAniListAPI()->get(API::sharedAPI()->sharedAniListAPI()->API_USER_LIST(this->displayName())).object();
  QJsonObject custom_list_data = user_list_data.value("custom_lists").toObject();
  user_list_data = user_list_data.value("lists").toObject();

  Anime *item;

  while(anime_list.count() > 0 && (item = anime_list.takeAt(0))) {
    delete item;
  }

  QMap<QString, QMap<QString, Anime*>> custom;

  QStringList list_names = user_list_data.keys();

  for(int i = 0; i < list_names.length(); i++) {
    QMap<QString, Anime*> list;

    for(QJsonValue ary : user_list_data.value(list_names.at(i)).toArray()) {
      Anime *anime_data = new Anime();
      QJsonObject anime = ary.toObject();
      QJsonObject inner_anime = anime.value("anime").toObject();

      anime_data->setID(     QString::number(inner_anime.value("id")              .toInt()));
      anime_data->setRomajiTitle(            inner_anime.value("title_romaji")    .toString());
      anime_data->setJapaneseTitle(          inner_anime.value("title_japanese")  .toString());
      anime_data->setEnglishTitle(           inner_anime.value("title_english")   .toString());
      anime_data->setType(                   inner_anime.value("type")            .toString());
      anime_data->setAiringStatus(           inner_anime.value("airing_status")   .toString());
      anime_data->setEpisodeCount(           inner_anime.value("total_episodes")  .toInt());
      anime_data->setAverageScore(           inner_anime.value("average_score")   .toString());
      anime_data->setCoverURL(          QUrl(inner_anime.value("image_url_lge")   .toString()));
      anime_data->setTitle(                  inner_anime.value(title_language)    .toString());

      anime_data->setMyProgress(             anime      .value("episodes_watched").toInt(0));
      anime_data->setMyNotes(                anime      .value("notes")           .toString());
      anime_data->setMyRewatch(              anime      .value("rewatched")       .toInt(0));
      anime_data->setMyStatus(               anime      .value("list_status")     .toString());

      QJsonArray synonyms = inner_anime.value("synonyms").toArray();

      for(int j = 0; j < synonyms.count(); j++) {
        anime_data->addSynonym(synonyms.at(j).toString());
      }

      if(scoreType() == 0 || scoreType() == 1) {
        anime_data->setMyScore(QString::number(anime    .value("score")           .toInt(0)));
      } else if(scoreType() == 4) {
        anime_data->setMyScore(QString::number(anime    .value("score")           .toDouble(0.0)));
      } else if(scoreType() == 2) {
        int scr = anime.value("score").toInt(0);
        QString my_score = QString::number(scr) + " ★";
        anime_data->setMyScore(my_score);
      } else {
        anime_data->setMyScore(                anime    .value("score")           .toString(""));
      }


      if(list.contains(anime_data->getID())) {
        Anime *old = list.value(anime_data->getID());
        anime_list.removeAll(old);
      }

      list.insert(anime_data->getID(), anime_data);

      anime_list.append(anime_data);
    }

    user_lists.insert(list_names.at(i), list);
  }

  QStringList custom_keys = custom_list_data.keys();

  for(int i = 0; i < custom_keys.length(); i++) {
    QMap<QString, Anime *> list;
    for(QJsonValue ary : custom_list_data.value(custom_keys.at(i)).toArray()) {
      Anime *anime_data = new Anime();
      QJsonObject anime = ary.toObject();
      QJsonObject inner_anime = anime.value("anime").toObject();

      anime_data->setID(     QString::number(inner_anime.value("id")              .toInt()));
      anime_data->setRomajiTitle(            inner_anime.value("title_romaji")    .toString());
      anime_data->setJapaneseTitle(          inner_anime.value("title_japanese")  .toString());
      anime_data->setEnglishTitle(           inner_anime.value("title_english")   .toString());
      anime_data->setType(                   inner_anime.value("type")            .toString());
      anime_data->setAiringStatus(           inner_anime.value("airing_status")   .toString());
      anime_data->setEpisodeCount(           inner_anime.value("total_episodes")  .toInt());
      anime_data->setAverageScore(           inner_anime.value("average_score")   .toString());
      anime_data->setCoverURL(          QUrl(inner_anime.value("image_url_lge")   .toString()));
      anime_data->setTitle(                  inner_anime.value(title_language)    .toString());

      anime_data->setMyProgress(             anime      .value("episodes_watched").toInt(0));
      anime_data->setMyNotes(                anime      .value("notes")           .toString());
      anime_data->setMyRewatch(              anime      .value("rewatched")       .toInt(0));
      anime_data->setMyStatus(               anime      .value("list_status")     .toString());

      QJsonArray synonyms = inner_anime.value("synonyms").toArray();

      for(int j = 0; j < synonyms.count(); j++) {
        anime_data->addSynonym(synonyms.at(j).toString());
      }

      if(scoreType() == 0 || scoreType() == 1) {
        anime_data->setMyScore(QString::number(anime    .value("score")           .toInt(0)));
      } else if(scoreType() == 4) {
        anime_data->setMyScore(QString::number(anime    .value("score")           .toDouble(0.0)));
      } else if(scoreType() == 2) {
        int scr = anime.value("score").toInt(0);
        QString my_score = QString::number(scr) + " ★";
        anime_data->setMyScore(my_score);
      } else {
        anime_data->setMyScore(                anime    .value("score")           .toString(""));
      }

      bool skip = false;

      for(Anime *a : anime_list) {
        if(a->getTitle() == anime_data->getTitle()) {
          delete anime_data;
          anime_data = a;
          skip = true;
        }
      }

      if(user_lists.value(anime_data->getMyStatus()).contains(anime_data->getID()) && !skip) {
        Anime *old = user_lists.value(anime_data->getMyStatus()).value(anime_data->getID());
        delete anime_data;
        anime_data = old;
      } else if(!skip){
        anime_list.append(anime_data);
      }

      list.insert(anime_data->getID(), anime_data);
    }

    custom.insert(this->customLists().at(custom_keys.at(i).toInt()).toString(), list);
  }

  for(int k = 0; k < customLists().length(); k++) {
    QString key = this->customLists().at(k).toString();
    user_lists.insert(key , custom.value(key));
  }

  qDebug() << "Loaded" << anime_list.count() << "anime";
}

Anime *User::getAnimeByTitle(QString title) {
  title = title.toLower();
  for(Anime *anime : anime_list) {
    if(anime->getEnglishTitle().toLower() == title || anime->getJapaneseTitle().toLower() == title ||  anime->getRomajiTitle().toLower() == title) {
      return anime;
    }

    for(QString synonym : anime->getSynonyms()) {
      if(title== synonym) {
        return anime;
      }
    }
  }

  return 0;
}

Anime *User::getAnimeByData(QString title, QString episodes, QString score, QString type) {
  title = title.toLower();
  for(Anime *anime : anime_list) {
    if(anime->getEnglishTitle().toLower() == title || anime->getJapaneseTitle().toLower() == title ||  anime->getRomajiTitle().toLower() == title) {
      if(QString::number(anime->getMyProgress()) + " / " + QString::number(anime->getEpisodeCount()) == episodes) {
        if(anime->getMyScore() == score) {
          if(anime->getType() == type) {
            return anime;
          }
        }
      }
    }
  }

  return 0;
}

void User::loadAnimeData(Anime *anime, bool download_cover) {
  QString ID = anime->getID();
  QUrl ID_URL = API::sharedAPI()->sharedAniListAPI()->API_ANIME(ID);

  QJsonObject result = API::sharedAPI()->sharedAniListAPI()->get(ID_URL).object();

  anime->setCoverURL(          QUrl(result.value("image_url_lge")   .toString()));

  if(download_cover) {
    if(anime->needsCover()) {
      anime->downloadCover();
    }
  }

  QString description = result.value("description").toString();

  anime->setDuration(               result.value("duration").toInt());
  anime->setSynopsis(               description);
  anime->setRomajiTitle(            result.value("title_romaji")    .toString());
  anime->setJapaneseTitle(          result.value("title_japanese")  .toString());
  anime->setEnglishTitle(           result.value("title_english")   .toString());
  anime->setType(                   result.value("type")            .toString());
  anime->setAiringStatus(           result.value("airing_status")   .toString());
  anime->setEpisodeCount(           result.value("total_episodes")  .toInt());
  anime->setAverageScore(           result.value("average_score")   .toString());
  anime->setTitle(                  result.value(title_language)    .toString());

  QJsonArray synonyms = result.value("synonyms").toArray();

  for(int j = 0; j < synonyms.count(); j++) {
    anime->addSynonym(synonyms.at(j).toString());
  }

  anime->finishReload();

  qDebug() << "Loaded extra data for anime" << anime->getTitle();
}

User* User::remake() {
  if(this->m_Instance) {
    delete m_Instance;
    m_Instance = 0;
  }

  return User::sharedUser();
}
