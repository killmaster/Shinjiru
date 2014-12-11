#ifndef ANITOMYWRAPPER_H
#define ANITOMYWRAPPER_H

#include <QString>
#include <QMap>

#include <lib/anitomy/anitomy/anitomy.h>

typedef std::basic_string<wchar_t> anitomy_string;

class AnitomyWrapper {
public:
  AnitomyWrapper();
  QMap<QString, QString> parse(QString);

private:
  anitomy::Anitomy anitomy;
  std::basic_string<wchar_t> toAnitomyFormat(QString);
};

#endif // ANITOMYWRAPPER_H
