/* Copyright 2015 Kazakuri */

#ifndef SRC_API_API_H_
#define SRC_API_API_H_

#include <QObject>
#include <QtConcurrent>

#include <anilistapi.h>

#include <functional>
#include <memory>
#include <vector>

class API : public QObject {
  Q_OBJECT

 public:
  static API* sharedAPI();
  AniListAPI* sharedAniListAPI();

  int verifyAPI();

 private:
  API();
  ~API();
  API(const API&);
  API& operator=(const API&);

  static API* m_Instance;
  AniListAPI* m_API;
  bool busy = false;
};

#endif  // SRC_API_API_H_
