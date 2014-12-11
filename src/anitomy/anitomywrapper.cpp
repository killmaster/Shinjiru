#include <QDebug>
#include <string>
#include <regex>

#include "anitomywrapper.h"

AnitomyWrapper::AnitomyWrapper() {
}

anitomy_string AnitomyWrapper::toAnitomyFormat(QString text) {
  const char* s = text.toLocal8Bit().constData();
  std::wstring w(s, s+strlen(s));
  return w.c_str();
}

QMap<QString, QString> AnitomyWrapper::parse(QString file_name) {
  anitomy_string title = toAnitomyFormat(file_name);
  QMap<QString, QString> data;

  try {
    anitomy.Parse(title);
  } catch(std::regex_error& e) {
    qDebug() << "Error parsing: " << QString::fromWCharArray(title.c_str());
    return QMap<QString, QString>();
  }

  auto& elements = anitomy.elements();

  data.insert("title",   QString::fromWCharArray(elements.get(anitomy::kElementAnimeTitle).c_str()));
  data.insert("episode", QString::fromWCharArray(elements.get(anitomy::kElementEpisodeNumber).c_str()));
  data.insert("subs",    QString::fromWCharArray(elements.get(anitomy::kElementReleaseGroup).c_str()));
  data.insert("res",     QString::fromWCharArray(elements.get(anitomy::kElementVideoResolution).c_str()));

  return data;
}
