#ifndef API_H
#define API_H

#include <QObject>
#include <anilistapi.h>

class API : public QObject {
  Q_OBJECT

public:
  API(QObject *parent = 0);
  int verify(AniListAPI *api);
};

#endif // API_H
