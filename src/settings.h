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
  QString parseEnum(int);
};

#endif // SETTINGS_H
