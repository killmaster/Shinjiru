/* Copyright 2015 Kazakuri */

#ifndef SRC_SETTINGS_H_
#define SRC_SETTINGS_H_

#include <QObject>
#include <QSettings>

class Settings : public QObject {
  Q_OBJECT

 public:
  explicit Settings(QObject *parent = 0);
  void setValue(int, QVariant);
  QVariant getValue(int, QVariant);

  enum SettingTypes {
    AnimeRecognitionEnabled,
    TorrentRefreshTime,
    TorrentRSSURL,
    DefaultRuleType,
    AniListExpires,
    AniListRefresh,
    AniListAccess,
    AutoUpdateDelay,
    StartOnBoot,
    MinimizeToTray,
    CloseToTray,
    ListOrder,
    DownloadCount,
    RuleCount,
    ReleaseStream
  };

 private:
  QSettings settings;
  QString parseEnum(int setting);
};

#endif  // SRC_SETTINGS_H_
