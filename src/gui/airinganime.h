/* Copyright 2015 Kazakuri */

#ifndef SRC_GUI_AIRINGANIME_H_
#define SRC_GUI_AIRINGANIME_H_

#include <QWidget>
#include <QPaintEvent>
#include <QMouseEvent>

#include "../api/anime.h"
#include "./scrolltext.h"

namespace Ui {
class AiringAnime;
}

class AiringAnime : public QWidget {
  Q_OBJECT

 public:
  explicit AiringAnime(QWidget *parent = 0, int scoreType = 0);
  ~AiringAnime();
  void setAnime(Anime *anime);
  Anime* getAnime();
  void tick();

 protected:
  void paintEvent(QPaintEvent *e);
  void mouseDoubleClickEvent(QMouseEvent *e);

 private:
  Ui::AiringAnime *ui;
  Anime *anime;
  ScrollText *text;
  int scoreType;
};

#endif  // SRC_GUI_AIRINGANIME_H_
