/* Copyright 2015 Kazakuri */

#ifndef SRC_ANITOMY_ANITOMYWRAPPER_H_
#define SRC_ANITOMY_ANITOMYWRAPPER_H_

#include <QString>
#include <QMap>

#include <string>

#include "../../lib/anitomy/anitomy/anitomy.h"


typedef std::basic_string<wchar_t> anitomy_string;

class AnitomyWrapper {
 public:
  AnitomyWrapper();
  QMap<QString, QString> parse(QString);

 private:
  anitomy::Anitomy anitomy;
  std::basic_string<wchar_t> toAnitomyFormat(QString);
};

#endif  // SRC_ANITOMY_ANITOMYWRAPPER_H_
