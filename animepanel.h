#ifndef ANIMEPANEL_H
#define ANIMEPANEL_H

#include <QDialog>
#include <QString>

#include "anime.h"

namespace Ui {
  class AnimePanel;
}

class AnimePanel : public QDialog {
  Q_OBJECT

public:
  explicit AnimePanel(QWidget *parent = 0, Anime *anime = 0);
  ~AnimePanel();
  bool loadNeeded = false;

private:
  Ui::AnimePanel *ui;
  Anime *anime;

private slots:
  void refreshDisplay();
};

#endif // ANIMEPANEL_H
