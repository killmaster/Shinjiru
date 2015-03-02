#ifndef SEASONANIME_H
#define SEASONANIME_H

#include <QWidget>
#include <QPaintEvent>
#include <QMouseEvent>

#include "../api/anime.h"
#include "scrolltext.h"

namespace Ui {
class SeasonAnime;
}

class SeasonAnime : public QWidget
{
  Q_OBJECT

public:
  explicit SeasonAnime(QWidget *parent = 0, int scoreType = 0);
  ~SeasonAnime();
  void setAnime(Anime *anime);
  Anime* getAnime();

protected:
  void paintEvent(QPaintEvent *);
  void mouseDoubleClickEvent(QMouseEvent *);

private:
  Ui::SeasonAnime *ui;
  Anime *anime;
  ScrollText *text;
  int scoreType;
};

#endif // SEASONANIME_H
