#ifndef ANIMEPANEL_H
#define ANIMEPANEL_H

#include <QDialog>
#include <QString>

#include "../api/api.h"

#include "../api/anime.h"

namespace Ui {
  class AnimePanel;
}

class AnimePanel : public QDialog {
  Q_OBJECT

public:
  explicit AnimePanel(QWidget *parent = 0, Anime *anime = 0, int score_type = 0);
  ~AnimePanel();
  bool loadNeeded = false;


protected:
  void paintEvent(QPaintEvent *);


private:
  Ui::AnimePanel *ui;
  Anime *anime;
  int score_type;
  QWidget *score_widget;

private slots:
  void refreshDisplay();
  void accept();
};

#endif // ANIMEPANEL_H
