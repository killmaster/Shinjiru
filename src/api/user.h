/* Copyright 2015 Kazakuri */

#ifndef SRC_API_USER_H_
#define SRC_API_USER_H_

#include <QPixmap>
#include <QMap>
#include <QVariantList>
#include <anilistapi.h>

#include <queue>

#include "./anime.h"
#include "../lib/filedownloader.h"
#include "./api.h"
#include "./smarttitle.h"

class User : public QObject{
  Q_OBJECT

 public:
  static User *sharedUser();
  static void deleteInstance();

 public slots:  // NOLINT
  QString displayName() const
    { return display_name; }
  int scoreType() const
    { return score_type; }
  QString profileImageURL() const
    { return profile_image_url; }
  QPixmap userImage() const
    { return user_image; }
  QMap<QString, QMap<QString, Anime*>> getUserList() const
    { return user_lists; }
  int animeTime() const
    { return anime_time; }
  QVariantList customLists() const
    { return custom_lists; }
  int notificationCount() const
    { return notifications; }

  User *remake();

  void setSmartTitles(QList<SmartTitle *> s);
  QList<SmartTitle *> getSmartTitles();
  void clearSmartTitles();

  Anime *resolveSmartTitle(QString title);
  Anime *getAnimeByTitle(QString title, bool smartTitle = false);
  Anime *getAnimeByData(QString title, QString episodes, QString score,
                        QString type);
  QList<Anime *> getAnimeList() const { return anime_list; }

  void setDisplayName(const QString &display_name)
    { this->display_name = display_name; }
  void setScoreType(const int &score_type);
  void setProfileImageURL(const QString &url)
    { this->profile_image_url = url; }
  void setUserImage(const QPixmap &user_image)
    { this->user_image = user_image; }
  void setUserImage(const QByteArray &data)
    { this->user_image.loadFromData(data); emit new_image();}
  void setTitleLanguage(const QString lang)
    { this->title_language = "title_" + lang; }
  void setAnimeTime(const int anime_time)
    { this->anime_time = anime_time; }
  void setCustomLists(const QVariantList list)
    { this->custom_lists = list; }
  void setNotificationCount(const int count)
    { this->notifications = count; }

  void fetchUpdatedList();
  void removeFromList(QString list, Anime *anime);
  void addToList(QString list, Anime *anime);
  void remove(Anime *anime);

  void loadUserList();
  void loadAnimeData(Anime *, bool);

  int loadNext();

  void animeChanged() {
    emit reloadRequested();
  }

  void quit() {
    this->cancel = true;

    for (QFuture<int> f : this->async_registry) {
      if (f.isRunning()) {
        qApp->processEvents();
        f.waitForFinished();
      }
    }

    emit quitFinished();
  }

  QByteArray listJson();

 signals:
  void new_image();
  void quitFinished();
  void reloadRequested();

 private:
  User();
  ~User();
  User(const User&);
  User& operator=(const User&);

  static User* m_Instance;

  QString display_name;
  int score_type;
  QString profile_image_url;
  QPixmap user_image;
  QString max_score;
  QMap<QString, QMap<QString, Anime*>> user_lists;
  QList<Anime*> anime_list;
  QString title_language;
  int anime_time;
  QVariantList custom_lists;
  int notifications;
  bool cancel;
  QList<SmartTitle *> smart_titles;

  std::queue<QMap<Anime *, bool>> queue;

  QList<QFuture<int>> async_registry;

  FileDownloader *user_image_control;

 private slots:  // NOLINT
  bool loadProfileImage();
};

#endif  // SRC_API_USER_H_
