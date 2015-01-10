#ifndef AIRINGANIME_H
#define AIRINGANIME_H

#include <QWidget>
#include <QPaintEvent>
#include <QMouseEvent>

#include "../api/anime.h"
#include "scrolltext.h"
#include "anilistapi.h"

namespace Ui {
class AiringAnime;
}

class AiringAnime : public QWidget
{
  Q_OBJECT

public:
  explicit AiringAnime(QWidget *parent = 0, int scoreType = 0, AniListAPI *api = 0);
  ~AiringAnime();
  void setAnime(Anime *anime);
  Anime* getAnime();

protected:
  void paintEvent(QPaintEvent *);
  void mouseDoubleClickEvent(QMouseEvent *);

private:
  Ui::AiringAnime *ui;
  Anime *anime;
  ScrollText *text;
  int scoreType;
  AniListAPI *api;
};

#endif // AIRINGANIME_H
