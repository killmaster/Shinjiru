#include <QDebug>
#include <string>
#include <regex>

#include "anitomywrapper.h"

AnitomyWrapper::AnitomyWrapper() {
}

anitomy_string AnitomyWrapper::toAnitomyFormat(QString text) {
  wchar_t *s = new wchar_t[text.length()];
  text.toWCharArray(s);

  return s;
}

QMap<QString, QString> AnitomyWrapper::parse(QString file_name) {
  anitomy_string title = toAnitomyFormat(file_name);
  QMap<QString, QString> data;

  try {
    anitomy.Parse(title);
  } catch(std::regex_error& e) {
    Q_UNUSED(e)
    qDebug() << "Error parsing: " << QString::fromWCharArray(title.c_str());
    return QMap<QString, QString>();
  }

  auto& elements = anitomy.elements();

  data.insert("title",   QString::fromWCharArray(elements.get(anitomy::kElementAnimeTitle).c_str()));
  data.insert("episode", QString::fromWCharArray(elements.get(anitomy::kElementEpisodeNumber).c_str()));
  data.insert("subs",    QString::fromWCharArray(elements.get(anitomy::kElementReleaseGroup).c_str()));

  QString res = QString::fromWCharArray(elements.get(anitomy::kElementVideoResolution).c_str());

  // Standardize the resolutions a bit instead of having 720p or 1280x720 available
  if(res.contains("x")) {
      res = res.split("x").last() + "p";
  } else if(res.contains("X")) {
      res = res.split("X").last() + "p";
  } else if(res.isEmpty()) {
      res = "720p"; // If we don't have a resolution, it's probably 720p so let's assume it is.
  }
  data.insert("res", res);

  return data;
}
