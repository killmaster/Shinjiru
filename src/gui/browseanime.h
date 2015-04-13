/* Copyright 2015 Kazakuri */

#ifndef SRC_GUI_BROWSEANIME_H_
#define SRC_GUI_BROWSEANIME_H_

#include <QWidget>
#include <QPaintEvent>
#include <QMouseEvent>

#include "../api/anime.h"
#include "./scrolltext.h"

namespace Ui {
class BrowseAnime;
}

class BrowseAnime : public QWidget {
  Q_OBJECT

 public:
  explicit BrowseAnime(QWidget *parent = 0, int scoreType = 0);
  ~BrowseAnime();
  void setAnime(Anime *anime);
  Anime* getAnime();

 public slots:  // NOLINT
  void reload();

 protected:
  void paintEvent(QPaintEvent *e);
  void mouseDoubleClickEvent(QMouseEvent *e);

 private:
  Ui::BrowseAnime *ui;
  Anime *anime;
  ScrollText *text;
  int scoreType;
};

#endif  // SRC_GUI_BROWSEANIME_H_
