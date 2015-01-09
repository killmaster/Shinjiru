#ifndef USER_H
#define USER_H

#include <QPixmap>
#include <QMap>

#include <anilistapi.h>

#include "anime.h"
#include "../lib/filedownloader.h"

class User : public QObject{
  Q_OBJECT

public:
  User(AniListAPI *api, QObject *parent = 0);
  ~User();

public slots:
  QString displayName()                              const { return display_name; }
  int scoreType()                                    const { return score_type; }
  QString profileImageURL()                          const { return profile_image_url; }
  QPixmap userImage()                                const { return user_image; }
  QMap<QString, QMap<QString, Anime*>> getUserList() const { return user_lists; }
  Anime *getAnimeByTitle(QString title);
  QList<Anime *> getAnimeList() const { return anime_list; }

  void setDisplayName(const QString &display_name) { this->display_name = display_name; }
  void setScoreType(const int &score_type);
  void setProfileImageURL(const QString &url)      { this->profile_image_url = url; }
  void setUserImage(const QPixmap &user_image)     { this->user_image = user_image; }
  void setUserImage(const QByteArray &data)        { this->user_image.loadFromData(data); emit new_image();}
  void setTitleLanguage(const QString lang)        { this->title_language = "title_" + lang; }
  void loadUserList();
  void loadAnimeData(Anime *, bool);

signals:
  void new_image();

private:
  AniListAPI *api;
  QString display_name;
  int score_type;
  QString profile_image_url;
  QPixmap user_image;
  QString max_score;
  QMap<QString, QMap<QString, Anime*>> user_lists;
  QList<Anime*> anime_list;
  QString title_language;

  FileDownloader *user_image_control;

private slots:
  bool loadProfileImage();
};

#endif // USER_H
