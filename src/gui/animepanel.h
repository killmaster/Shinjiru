/* Copyright 2015 Kazakuri */

#ifndef SRC_GUI_ANIMEPANEL_H_
#define SRC_GUI_ANIMEPANEL_H_

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
  explicit AnimePanel(QWidget *parent = 0, Anime *anime = 0, int st = 0);
  ~AnimePanel();
  bool loadNeeded = false;


 protected:
  void paintEvent(QPaintEvent *e);


 private:
  Ui::AnimePanel *ui;
  Anime *anime;
  int score_type;
  QWidget *score_widget;
  bool new_entry;

 private slots:  // NOLINT
  void refreshDisplay();
  void accept();
};

#endif  // SRC_GUI_ANIMEPANEL_H_
