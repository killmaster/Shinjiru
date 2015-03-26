#ifndef BrowseAnime_H
#define BrowseAnime_H

#include <QWidget>
#include <QPaintEvent>
#include <QMouseEvent>

#include "../api/anime.h"
#include "scrolltext.h"

namespace Ui {
class BrowseAnime;
}

class BrowseAnime : public QWidget
{
  Q_OBJECT

public:
  explicit BrowseAnime(QWidget *parent = 0, int scoreType = 0);
  ~BrowseAnime();
  void setAnime(Anime *anime);
  Anime* getAnime();

protected:
  void paintEvent(QPaintEvent *);
  void mouseDoubleClickEvent(QMouseEvent *);

private:
  Ui::BrowseAnime *ui;
  Anime *anime;
  ScrollText *text;
  int scoreType;
};

#endif // BrowseAnime_H
