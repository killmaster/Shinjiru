/* Copyright 2015 Kazakuri */

#include "./settings.h"

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
  switch (setting) {
    case AnimeRecognitionEnabled:
      return "recognition";
    case DefaultRuleType:
      return "rule_type";
    case TorrentRefreshTime:
      return "torrent_interval";
    case TorrentRSSURL:
      return "rss_url";
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
    case ReleaseStream:
      return "release_stream";
    case CheckUpdates:
      return "update_on_boot";
    case StartMinimized:
      return "start_minimized";
    case AnimeDetectNotify:
      return "anime_detect_notify";
    case AnimeUpdateNotify:
      return "anime_update_notify";
    case TorrentsEnabled:
      return "torrents_enabled";
    case AutoDownload:
      return "auto_download";
    case AutoNotify:
      return "auto_notify";
    case UserRefreshTime:
      return "user_refresh_time";
  }

  return "unknown";
}
