#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QSettings>

class Settings : public QObject {
  Q_OBJECT
public:
  Settings(QObject *parent = 0);
  void setValue(int, QVariant);
  QVariant getValue(int, QVariant);

  enum SettingTypes {
    AnimeRecognitionEnabled,
    TorrentRefreshTime,
    DefaultRuleType,
    AniListExpires,
    AniListRefresh,
    AniListAccess,
    AutoUpdateDelay,
    StartOnBoot,
    MinimizeToTray,
    CloseToTray
  };

private:
  QSettings settings;
  QString parseEnum(int);
};

#endif // SETTINGS_H
