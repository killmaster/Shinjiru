/* Copyright 2015 Kazakuri */

#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QEventLoop>
#include <QMessageBox>

#include "./user.h"
#include "./anime.h"

User* User::m_Instance = 0;

User* User::sharedUser() {
  static QMutex mutex;
  if (!m_Instance) {
    mutex.lock();

    if (!m_Instance) {
      m_Instance = new User;
    }

    mutex.unlock();
  }

  return m_Instance;
}

void User::deleteInstance() {
  static QMutex mutex;
  mutex.lock();
  delete m_Instance;
  m_Instance = 0;
  mutex.unlock();
}

User::User() : QObject(0) {
  QUrl user = API::sharedAPI()->sharedAniListAPI()->API_USER;
  QJsonObject result = API::sharedAPI()->sharedAniListAPI()->get(user).object();

  user_image_control = nullptr;

  this->setDisplayName(result.value("display_name").toString());
  this->setScoreType(result.value("score_type").toInt());
  this->setProfileImageURL(result.value("image_url_med").toString());
  this->setTitleLanguage(result.value("title_language").toString());
  this->setAnimeTime(result.value("anime_time").toInt());
  this->setNotificationCount(result.value("notifications").toInt());
  this->setCustomLists(result.value("custom_list_anime").toArray()
                       .toVariantList());

  this->cancel = false;

  this->loadProfileImage();
}

User::~User() {
  this->clearSmartTitles();
  for (Anime *a : anime_list) {
    delete a;
  }
}

bool User::loadProfileImage() {
  if (profileImageURL().isEmpty()) return false;

  QUrl url(profileImageURL());

  qDebug() << "Downloading user image" << profileImageURL()
           << "for" << displayName();

  user_image_control = new FileDownloader(url);

  QEventLoop event;
  connect(user_image_control, SIGNAL(downloaded()), &event, SLOT(quit()));
  event.exec();

  this->setUserImage(user_image_control->downloadedData());

  user_image_control->deleteLater();
  user_image_control = nullptr;

  return true;
}

void User::setScoreType(const int &score_type) {
  this->score_type = score_type;

  switch (score_type) {
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
  }
}

void User::loadUserList() {
  qDebug() << "Fetching user list";

  QJsonObject user_list_data =
      API::sharedAPI()->sharedAniListAPI()->get
        (API::sharedAPI()->sharedAniListAPI()->API_USER_LIST
          (this->displayName())).object();

  QJsonObject custom_list_data =
      user_list_data.value("custom_lists").toObject();

  if (user_list_data.value("custom_lists").isArray()) {
    QJsonArray ar = user_list_data.value("custom_lists").toArray();

    for (int i = 0; i < ar.count(); i++) {
      custom_list_data.insert(QString::number(i), ar.at(i));
    }
  }

  user_list_data = user_list_data.value("lists").toObject();

  Anime *item;

  while (anime_list.count() > 0 && (item = anime_list.takeAt(0))) {
    delete item;
  }

  QMap<QString, QMap<QString, Anime*>> custom;

  QStringList list_names = user_list_data.keys();

  for (int i = 0; i < list_names.length(); i++) {
    QMap<QString, Anime*> list;

    for (QJsonValue ary : user_list_data.value(list_names.at(i)).toArray()) {
      Anime *anime_data = new Anime();
      QJsonObject anime = ary.toObject();
      QJsonObject inner_anime = anime.value("anime").toObject();

      anime_data->setID(QString::number(inner_anime.value("id").toInt()));
      anime_data->setRomajiTitle(inner_anime.value("title_romaji").toString());
      anime_data->setJapaneseTitle(inner_anime.value("title_japanese")
                                   .toString());
      anime_data->setEnglishTitle(inner_anime.value("title_english")
                                  .toString());
      anime_data->setType(inner_anime.value("type").toString());
      anime_data->setAiringStatus(inner_anime.value("airing_status")
                                  .toString());
      anime_data->setEpisodeCount(inner_anime.value("total_episodes").toInt());
      anime_data->setAverageScore(inner_anime.value("average_score")
                                  .toString());
      anime_data->setCoverURL(QUrl(inner_anime.value("image_url_lge")
                                   .toString()));
      anime_data->setTitle(inner_anime.value(title_language).toString());

      anime_data->setMyProgress(anime.value("episodes_watched").toInt(0));
      anime_data->setMyNotes(anime.value("notes").toString());
      anime_data->setMyRewatch(anime.value("rewatched").toInt(0));
      anime_data->setMyStatus(anime.value("list_status").toString());
      anime_data->setDefaultHidden(anime.value("hidden_default").toInt());

      QVariantList v = anime.value("custom_lists").toArray().toVariantList();

      QList<int> custom_lists;

      for (int i = 0; i < v.length(); i++) {
        custom_lists.append(v.at(i).toInt());
      }

      anime_data->setCustomLists(custom_lists);

      QJsonArray synonyms = inner_anime.value("synonyms").toArray();

      for (int j = 0; j < synonyms.count(); j++) {
        anime_data->addSynonym(synonyms.at(j).toString());
      }

      if (scoreType() == 0 || scoreType() == 1) {
        anime_data->setMyScore(QString::number(anime.value("score").toInt(0)));
      } else if (scoreType() == 4) {
        anime_data->setMyScore(QString::number(
                                 anime.value("score").toDouble(0.0)));
      } else if (scoreType() == 2) {
        int scr = anime.value("score").toInt(0);
        QString my_score = QString::number(scr) + " ★";
        anime_data->setMyScore(my_score);
      } else {
        anime_data->setMyScore(anime.value("score").toString(""));
      }

      if (list.contains(anime_data->getID())) {
        Anime *old = list.value(anime_data->getID());
        anime_list.removeAll(old);
      }

      list.insert(anime_data->getID(), anime_data);

      anime_list.append(anime_data);
    }

    user_lists.insert(list_names.at(i), list);
  }

  QStringList custom_keys = custom_list_data.keys();

  for (int i = 0; i < custom_keys.length(); i++) {
    QMap<QString, Anime *> list;
    for (QJsonValue ary : custom_list_data.value(custom_keys.at(i)).toArray()) {
      Anime *anime_data = new Anime();
      QJsonObject anime = ary.toObject();
      QJsonObject inner_anime = anime.value("anime").toObject();

      anime_data->setID(QString::number(inner_anime.value("id").toInt()));
      anime_data->setRomajiTitle(inner_anime.value("title_romaji").toString());
      anime_data->setJapaneseTitle(inner_anime.value("title_japanese")
                                   .toString());
      anime_data->setEnglishTitle(inner_anime.value("title_english")
                                  .toString());
      anime_data->setType(inner_anime.value("type").toString());
      anime_data->setAiringStatus(inner_anime.value("airing_status")
                                  .toString());
      anime_data->setEpisodeCount(inner_anime.value("total_episodes").toInt());
      anime_data->setAverageScore(inner_anime.value("average_score")
                                  .toString());
      anime_data->setCoverURL(QUrl(inner_anime.value("image_url_lge")
                                   .toString()));
      anime_data->setTitle(inner_anime.value(title_language).toString());

      anime_data->setMyProgress(anime.value("episodes_watched").toInt(0));
      anime_data->setMyNotes(anime.value("notes").toString());
      anime_data->setMyRewatch(anime.value("rewatched").toInt(0));
      anime_data->setMyStatus(anime.value("list_status").toString());
      anime_data->setDefaultHidden(anime.value("hidden_default").toInt());

      QVariantList v = anime.value("custom_lists").toArray().toVariantList();

      QList<int> custom_lists;

      for (int i = 0; i < v.length(); i++) {
        custom_lists.append(v.at(i).toInt());
      }

      anime_data->setCustomLists(custom_lists);

      QJsonArray synonyms = inner_anime.value("synonyms").toArray();

      for (int j = 0; j < synonyms.count(); j++) {
        anime_data->addSynonym(synonyms.at(j).toString());
      }

      if (scoreType() == 0 || scoreType() == 1) {
        anime_data->setMyScore(QString::number(anime.value("score").toInt(0)));
      } else if (scoreType() == 4) {
        anime_data->setMyScore(QString::number(anime.value("score")
                                               .toDouble(0.0)));
      } else if (scoreType() == 2) {
        int scr = anime.value("score").toInt(0);
        QString my_score = QString::number(scr) + " ★";
        anime_data->setMyScore(my_score);
      } else {
        anime_data->setMyScore(anime.value("score").toString(""));
      }

      bool skip = false;

      for (Anime *a : anime_list) {
        if (a->getTitle() == anime_data->getTitle()) {
          delete anime_data;
          anime_data = a;
          skip = true;
        }
      }

      if (user_lists.value(anime_data->getMyStatus())
          .contains(anime_data->getID()) && !skip) {
        Anime *old =
            user_lists.value(anime_data->getMyStatus())
            .value(anime_data->getID());
        delete anime_data;
        anime_data = old;
      } else if (!skip) {
        anime_list.append(anime_data);
      }

      list.insert(anime_data->getID(), anime_data);
    }

    custom.insert(this->customLists().at(custom_keys.at(i).toInt()).toString(),
                  list);
  }

  for (int k = 0; k < customLists().length(); k++) {
    QString key = this->customLists().at(k).toString();
    user_lists.insert(key , custom.value(key));
  }

  qDebug() << "Loaded" << anime_list.count() << "anime";
}

Anime *User::getAnimeByTitle(QString title, bool smartTitle) {
  title = title.toLower();

  if (smartTitle) {
    Anime* a = resolveSmartTitle(title);

    if (a != nullptr) return a;
  }

  for (Anime *anime : anime_list) {
    if (anime->getEnglishTitle().toLower() == title ||
        anime->getJapaneseTitle().toLower() == title ||
        anime->getRomajiTitle().toLower() == title) {
      return anime;
    }

    for (QString synonym : anime->getSynonyms()) {
      if (title == synonym.toLower()) {
        return anime;
      }
    }
  }

  return nullptr;
}

Anime *User::getAnimeByData(QString title, QString episodes, QString score,
                            QString type) {
  title = title.toLower();
  for (Anime *anime : anime_list) {
    if (anime->getEnglishTitle().toLower() == title ||
        anime->getJapaneseTitle().toLower() == title ||
        anime->getRomajiTitle().toLower() == title) {
      if (QString::number(anime->getMyProgress()) + " / " +
          QString::number(anime->getEpisodeCount()) == episodes) {
        if (anime->getMyScore() == score) {
          if (anime->getType() == type) {
            return anime;
          }
        }
      }
    }
  }

  return getAnimeByTitle(title);
}

void User::loadAnimeData(Anime *anime, bool download_cover) {
  int queue_size = queue.size();

  QMap<Anime *, bool> data;
  data.insert(anime, download_cover);
  queue.push(data);

  if (queue_size == 0) {
    async_registry.append(QtConcurrent::run([&] () { // NOLINT
      return loadNext();
    }));
  }
}

int User::loadNext() {
  if (queue.size() == 0) return 1;
  if (this->cancel) return 1;

  QMap<Anime *, bool> data = queue.front();
  queue.pop();

  Anime *anime = data.keys().first();
  bool download_cover = data.values().first();

  QString ID = anime->getID();
  QUrl ID_URL = API::sharedAPI()->sharedAniListAPI()->API_ANIME(ID);

  QJsonObject result =
      API::sharedAPI()->sharedAniListAPI()->get(ID_URL).object();

  anime->setCoverURL(QUrl(result.value("image_url_lge").toString()));

  if (download_cover) {
    QEventLoop evt;
    connect(anime, SIGNAL(new_image()), &evt, SLOT(quit()));
    anime->downloadCover();
    evt.exec();
  }

  QString description = result.value("description").toString();

  anime->setDuration(result.value("duration").toInt());
  anime->setSynopsis(description);
  anime->setRomajiTitle(result.value("title_romaji").toString());
  anime->setJapaneseTitle(result.value("title_japanese").toString());
  anime->setEnglishTitle(result.value("title_english").toString());
  anime->setType(result.value("type").toString());
  anime->setAiringStatus(result.value("airing_status").toString());
  anime->setEpisodeCount(result.value("total_episodes").toInt());
  anime->setAverageScore(result.value("average_score").toString());
  anime->setTitle(result.value(title_language).toString());

  if (anime->getAiringStatus() == "currently airing") {
    QJsonObject airing = result.value("airing").toObject();

    anime->setNextEpisode(airing.value("next_episode").toInt());
    anime->setCountdown(airing.value("countdown").toInt());

    if (anime->getCountdown() > 0) {
      anime->setAiringSchedule(true);
    } else {
      anime->setAiringSchedule(false);
    }
  }

  QJsonArray synonyms = result.value("synonyms").toArray();

  for (int j = 0; j < synonyms.count(); j++) {
    anime->addSynonym(synonyms.at(j).toString());
  }

  anime->finishReload();

  qDebug() << "Loaded extra data for anime" << anime->getTitle();

  if (queue.size() > 0) {
    async_registry.append(QtConcurrent::run([&]() { // NOLINT
      loadNext();
      return 1;
    }));
  }

  return 1;
}

User* User::remake() {
  QJsonObject result = API::sharedAPI()->sharedAniListAPI()->get
      (API::sharedAPI()->sharedAniListAPI()->API_USER).object();

  QString profile_image = result.value("image_url_med").toString();

  this->setDisplayName(result.value("display_name").toString());
  this->setScoreType(result.value("score_type").toInt());
  this->setTitleLanguage(result.value("title_language").toString());
  this->setAnimeTime(result.value("anime_time").toInt());
  this->setCustomLists(result.value("custom_list_anime").toArray()
                       .toVariantList());
  this->setNotificationCount(result.value("notifications").toInt());

  if (this->profile_image_url != profile_image) {
    this->setProfileImageURL(profile_image);
    this->loadProfileImage();
  }

  this->fetchUpdatedList();

  return User::sharedUser();
}

void User::fetchUpdatedList() {
  qDebug() << "Fetching updated user list";

  QJsonObject user_list_data =
      API::sharedAPI()->sharedAniListAPI()->get(
        API::sharedAPI()->sharedAniListAPI()->API_USER_LIST(
          this->displayName())).object();

  QJsonObject custom_list_data =
      user_list_data.value("custom_lists").toObject();

  if (user_list_data.value("custom_lists").isArray()) {
    QJsonArray ar = user_list_data.value("custom_lists").toArray();

    for (int i = 0; i < ar.count(); i++) {
      custom_list_data.insert(QString::number(i), ar.at(i));
    }
  }

  user_list_data = user_list_data.value("lists").toObject();

  QMap<QString, QMap<QString, Anime*>> custom;

  QStringList list_names = user_list_data.keys();

  for (int i = 0; i < list_names.length(); i++) {
    QMap<QString, Anime*> list = user_lists.value(list_names.at(i));

    for (QJsonValue ary : user_list_data.value(list_names.at(i)).toArray()) {
      QJsonObject anime = ary.toObject();
      QJsonObject inner_anime = anime.value("anime").toObject();

      QString id = QString::number(inner_anime.value("id").toInt());
      Anime *anime_data;

      if (list.contains(id)) {
        anime_data = list.value(id);
      } else {
        anime_data = new Anime();
      }

      anime_data->setID(QString::number(inner_anime.value("id").toInt()));
      anime_data->setRomajiTitle(inner_anime.value("title_romaji").toString());
      anime_data->setJapaneseTitle(inner_anime.value("title_japanese")
                                   .toString());
      anime_data->setEnglishTitle(inner_anime.value("title_english")
                                  .toString());
      anime_data->setType(inner_anime.value("type").toString());
      anime_data->setAiringStatus(inner_anime.value("airing_status")
                                  .toString());
      anime_data->setEpisodeCount(inner_anime.value("total_episodes")
                                  .toInt());
      anime_data->setAverageScore(inner_anime.value("average_score")
                                  .toString());
      anime_data->setCoverURL(QUrl(inner_anime.value("image_url_lge")
                                   .toString()));
      anime_data->setTitle(inner_anime.value(title_language).toString());

      anime_data->setMyProgress(anime.value("episodes_watched").toInt(0));
      anime_data->setMyNotes(anime.value("notes").toString());
      anime_data->setMyRewatch(anime.value("rewatched").toInt(0));
      anime_data->setMyStatus(anime.value("list_status").toString());
      anime_data->setDefaultHidden(anime.value("hidden_default").toInt());

      QVariantList v = anime.value("custom_lists").toArray().toVariantList();

      QList<int> custom_lists;

      for (int i = 0; i < v.length(); i++) {
        custom_lists.append(v.at(i).toInt());
      }

      anime_data->setCustomLists(custom_lists);

      QJsonArray synonyms = inner_anime.value("synonyms").toArray();

      for (int j = 0; j < synonyms.count(); j++) {
        anime_data->addSynonym(synonyms.at(j).toString());
      }

      if (scoreType() == 0 || scoreType() == 1) {
        anime_data->setMyScore(QString::number(anime.value("score").toInt(0)));
      } else if (scoreType() == 4) {
        anime_data->setMyScore(
              QString::number(anime.value("score").toDouble(0.0)));
      } else if (scoreType() == 2) {
        int scr = anime.value("score").toInt(0);
        QString my_score = QString::number(scr) + " ★";
        anime_data->setMyScore(my_score);
      } else {
        anime_data->setMyScore(anime.value("score").toString(""));
      }

      list.insert(anime_data->getID(), anime_data);

      if (!anime_list.contains(anime_data))
        anime_list.append(anime_data);
    }

    user_lists.insert(list_names.at(i), list);
  }

  QStringList custom_keys = custom_list_data.keys();

  for (int i = 0; i < custom_keys.length(); i++) {
    QMap<QString, Anime *> list =
        user_lists.value(this->customLists().at
                         (custom_keys.at(i).toInt()).toString());

    for (QJsonValue ary : custom_list_data.value(custom_keys.at(i)).toArray()) {
      QJsonObject anime = ary.toObject();
      QJsonObject inner_anime = anime.value("anime").toObject();

      QString id = QString::number(inner_anime.value("id").toInt());
      Anime *anime_data;

      if (list.contains(id)) {
        anime_data = list.value(id);
      } else {
        anime_data = new Anime();
      }

      anime_data->setID(QString::number(inner_anime.value("id").toInt()));
      anime_data->setRomajiTitle(inner_anime.value("title_romaji").toString());
      anime_data->setJapaneseTitle(inner_anime.value("title_japanese")
                                   .toString());
      anime_data->setEnglishTitle(inner_anime.value("title_english")
                                   .toString());
      anime_data->setType(inner_anime.value("type").toString());
      anime_data->setAiringStatus(inner_anime.value("airing_status")
                                  .toString());
      anime_data->setEpisodeCount(inner_anime.value("total_episodes").toInt());
      anime_data->setAverageScore(inner_anime.value("average_score")
                                  .toString());
      anime_data->setCoverURL(QUrl(inner_anime.value("image_url_lge")
                                   .toString()));
      anime_data->setTitle(inner_anime.value(title_language).toString());

      anime_data->setMyProgress(anime.value("episodes_watched").toInt(0));
      anime_data->setMyNotes(anime.value("notes").toString());
      anime_data->setMyRewatch(anime.value("rewatched").toInt(0));
      anime_data->setMyStatus(anime.value("list_status").toString());
      anime_data->setDefaultHidden(anime.value("hidden_default").toInt());

      QVariantList v = anime.value("custom_lists").toArray().toVariantList();

      QList<int> custom_lists;

      for (int i = 0; i < v.length(); i++) {
        custom_lists.append(v.at(i).toInt());
      }

      anime_data->setCustomLists(custom_lists);

      QJsonArray synonyms = inner_anime.value("synonyms").toArray();

      for (int j = 0; j < synonyms.count(); j++) {
        anime_data->addSynonym(synonyms.at(j).toString());
      }

      if (scoreType() == 0 || scoreType() == 1) {
        anime_data->setMyScore(QString::number(anime .value("score").toInt(0)));
      } else if (scoreType() == 4) {
        anime_data->setMyScore(QString::number(anime.value("score")
                                               .toDouble(0.0)));
      } else if (scoreType() == 2) {
        int scr = anime.value("score").toInt(0);
        QString my_score = QString::number(scr) + " ★";
        anime_data->setMyScore(my_score);
      } else {
        anime_data->setMyScore(anime.value("score").toString(""));
      }

      list.insert(anime_data->getID(), anime_data);

      if (!anime_list.contains(anime_data))
        anime_list.append(anime_data);
    }

    custom.insert(this->customLists().at(custom_keys.at(i).toInt()).toString(),
                  list);
  }

  for (int k = 0; k < customLists().length(); k++) {
    QString key = this->customLists().at(k).toString();
    user_lists.insert(key , custom.value(key));
  }
}

void User::removeFromList(QString list, Anime *anime) {
  list = list.replace("-", "_");
  list = list.replace(" ", "_");

  if (list == "") return;

  QMap<QString, Anime*> user_list = user_lists.value(list);

  if (user_list.contains(anime->getID())) {
    user_list.remove(anime->getID());
    user_lists.insert(list, user_list);
  }
}

void User::addToList(QString list, Anime *anime) {
  list = list.replace("-", "_");
  list = list.replace(" ", "_");

  QMap<QString, Anime*> user_list = user_lists.value(list);

  if (!user_list.contains(anime->getID())) {
    user_list.insert(anime->getID(), anime);
    user_lists.insert(list, user_list);
  }

  if (!anime_list.contains(anime)) {
    anime_list.append(anime);
  }
}

void User::remove(Anime *anime) {
  for (QString key : user_lists.keys()) {
    QMap<QString, Anime*> m = user_lists.value(key);
    if (m.contains(anime->getID())) {
      m.remove(anime->getID());
      user_lists.insert(key, m);
    }
  }

  anime_list.removeAll(anime);

  delete anime;
}

QByteArray User::listJson() {
  return API::sharedAPI()->sharedAniListAPI()->get
      (API::sharedAPI()->sharedAniListAPI()->API_USER_LIST(
         this->displayName())).toJson();
}

void User::setSmartTitles(QList<SmartTitle *> s) {
  this->smart_titles = s;
}

QList<SmartTitle *> User::getSmartTitles() {
  return this->smart_titles;
}

void User::clearSmartTitles() {
  for (SmartTitle *s : smart_titles) {
    delete s;
  }

  smart_titles.clear();
}

Anime *User::resolveSmartTitle(QString title) {
  for (SmartTitle *s : smart_titles) {
    if (s->contains(title)) {
      Anime *a = nullptr;

      for (int i = 0; i < this->anime_list.count(); i++) {
        if (anime_list.at(i)->getID() == s->getID()) {
          a = anime_list.at(i);
        }
      }

      if (a != nullptr) {
        a->setUpdateOffset(0);

        if (s->hasOffset()) {
          a->setUpdateOffset(s->getOffset());
        }
        return a;
      }
    }
  }

  return nullptr;
}
