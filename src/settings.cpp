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
  }

  return "unknown";
}
