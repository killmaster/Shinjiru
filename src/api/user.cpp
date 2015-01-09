#include "user.h"
#include "anime.h"

#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QEventLoop>
#include <QtConcurrent>

User::User(AniListAPI *api, QObject *parent) : QObject(parent) {
  this->api = api;
  QJsonObject result = api->get(api->API_USER);

  user_image_control = nullptr;

  this->setDisplayName    (result.value("display_name")  .toString());
  this->setScoreType      (result.value("score_type")    .toInt());
  this->setProfileImageURL(result.value("image_url_med") .toString());
  this->setTitleLanguage  (result.value("title_language").toString());

  this->loadProfileImage();
}

User::~User() {
  if(user_image_control != nullptr) delete user_image_control;
}

bool User::loadProfileImage() {
  if(profileImageURL().isEmpty()) return false;

  QUrl url(profileImageURL());

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
  QJsonObject user_list_data = api->get(api->API_USER_LIST(this->displayName()));
  user_list_data = user_list_data.value("lists").toObject();

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
      anime_data->setMyScore(QString::number(anime      .value("score")           .toInt(0)));

      if(list.contains(anime_data->getID())){
        Anime *old = list.value(anime_data->getID());
        anime_list.removeAll(old);
      }

      list.insert(anime_data->getID(), anime_data);
      anime_list.append(anime_data);
    }

    user_lists.insert(list_names.at(i), list);
  }
}

Anime *User::getAnimeByTitle(QString title) {
  for(Anime *anime : anime_list) {
    if(anime->getEnglishTitle() == title || anime->getJapaneseTitle() == title ||  anime->getRomajiTitle() == title) {
      return anime;
    }

    for(QString synonym : anime->getSynonyms()) {
      if(title== synonym) {
        return anime;
      }
    }
  }

  return new Anime();
}

void User::loadAnimeData(Anime *anime, bool download_cover) {
  QString ID = anime->getID();
  QUrl ID_URL = api->API_ANIME(ID);

  QJsonObject result = api->get(ID_URL);

  if(download_cover) {
    anime->downloadCover();
  }

  QJsonArray synonyms = result.value("synonyms").toArray();
  QString description = result.value("description").toString();

  anime->setDuration(result.value("duration").toInt());

  for(int i = 0; i < synonyms.count(); i++) {
    anime->addSynonym(synonyms.at(i).toString());
  }

  anime->setSynopsis(description);

  anime->finishReload();

}
