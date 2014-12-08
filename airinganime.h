#ifndef AIRINGANIME_H
#define AIRINGANIME_H

#include <QWidget>

#include "anime.h"

#include <QPaintEvent>

#include "scrolltext.h"

namespace Ui {
class AiringAnime;
}

class AiringAnime : public QWidget
{
  Q_OBJECT

public:
  explicit AiringAnime(QWidget *parent = 0);
  ~AiringAnime();
  void setAnime(Anime *anime);
  Anime *anime;
  ScrollText *text;

protected:
  void paintEvent(QPaintEvent *);

private:
  Ui::AiringAnime *ui;
};

#endif // AIRINGANIME_H
