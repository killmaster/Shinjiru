/* Copyright 2015 Kazakuri */

#include <QDebug>

#include <string>
#include <regex>  //NOLINT

#include "./anitomywrapper.h"

AnitomyWrapper::AnitomyWrapper() {
}

anitomy_string AnitomyWrapper::toAnitomyFormat(QString text) {
  return text.toStdWString();
}

QMap<QString, QString> AnitomyWrapper::parse(QString file_name) {
  anitomy_string title = toAnitomyFormat(file_name);

  QMap<QString, QString> data;

  try {
    anitomy.Parse(title);
  } catch(std::regex_error& e) {
    Q_UNUSED(e)
    qWarning() << "Error parsing: " << QString::fromWCharArray(title.c_str());
    return QMap<QString, QString>();
  }

  auto& elements = anitomy.elements();

  data.insert("title",   QString::fromWCharArray(
                elements.get(anitomy::kElementAnimeTitle).c_str()));
  data.insert("episode", QString::fromWCharArray(
                elements.get(anitomy::kElementEpisodeNumber).c_str()));
  data.insert("subs",    QString::fromWCharArray(
                elements.get(anitomy::kElementReleaseGroup).c_str()));

  QString res = QString::fromWCharArray(
        elements.get(anitomy::kElementVideoResolution).c_str());

  // Standardize the resolutions a bit
  if (res.contains("x")) {
      res = res.split("x").last() + "p";
  } else if (res.contains("X")) {
      res = res.split("X").last() + "p";
  } else if (res.isEmpty()) {
      res = "720p";  // Let's assume it is 720p
  }
  data.insert("res", res);

  return data;
}
