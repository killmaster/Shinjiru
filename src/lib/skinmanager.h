/* Copyright 2015 Kazakuri */

#ifndef SRC_LIB_SKINMANAGER_H_
#define SRC_LIB_SKINMANAGER_H_

#include <QObject>
#include <QMap>

class SkinManager : public QObject {
  Q_OBJECT

 public:
  static SkinManager* sharedManager();
  QString get(int i);

  enum SkinElements {
    AboutMascot,
    ApplicationIcon,
    AnimeListIcon,
    AdvancedIcon,
    RecognitionIcon,
    TorrentsIcon
  };

 private:
  QMap<int, QString> element_paths;
  static SkinManager* m_Instance;


  SkinManager();
  ~SkinManager();
  SkinManager(const SkinManager &s);
  SkinManager& operator=(const SkinManager &s);
};

#endif  // SRC_LIB_SKINMANAGER_H_
