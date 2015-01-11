#ifndef API_H
#define API_H

#include <QObject>
#include <anilistapi.h>
#include <functional>
#include <QtConcurrent>
#include <memory>
#include <vector>

class API : public QObject {
  Q_OBJECT

public:
  static API* sharedAPI();
  AniListAPI* sharedAniListAPI();

  int verify();

private:
  API();
  ~API();
  API(const API&);
  API& operator=(const API&);

  static API* m_Instance;
  AniListAPI* m_API;
  bool busy = false;
};

#endif // API_H
