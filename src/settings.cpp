#include "settings.h"

Settings::Settings(QObject *parent): QObject(parent) {
}

QVariant Settings::getValue(int setting, QVariant default_value) {
  QString key = parseEnum(setting);
  return settings.value(key, default_value);
}

void Settings::setValue(int setting, QVariant value) {
  QString key = parseEnum(setting);
  settings.setValue(key, value);
}

QString Settings::parseEnum(int setting) {
  switch(setting) {
    case AnimeRecognitionEnabled:
      return "recognition";
    case DefaultRuleType:
      return "rule_type";
    case TorrentRefreshTime:
      return "torrent_interval";
    case AniListAccess:
      return "anilist/access";
    case AniListExpires:
      return "anilist/expires";
    case AniListRefresh:
      return "anilist/refresh";
    case AutoUpdateDelay:
      return "update_delay";
    case StartOnBoot:
      return "start_on_boot";
    case MinimizeToTray:
      return "minimize_to_tray";
    case CloseToTray:
      return "close_to_tray";
    case ListOrder:
      return "list_order";
    case DownloadCount:
      return "download_total";
    case RuleCount:
      return "rule_total";
  }

  return "unknown";
}
