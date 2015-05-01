/* Copyright 2015 Kazakuri */

#ifndef SRC_GUI_OVERLAY_H_
#define SRC_GUI_OVERLAY_H_

#include <QWidget>
#include <QPixmap>
#include <QMap>

class Overlay : public QWidget {
 public:
  explicit Overlay(QWidget *parent);
  void addDrawing(QString s, QPixmap *p) {
    removeDrawing(s);
    drawList.insert(s, p);
    update();
  }
  void removeDrawing(QString s) {
    if (drawList.contains(s))
      delete drawList.take(s);

    update();
  }

  void removeAll() {
    for (QString key : drawList.keys()) {
      removeDrawing(key);
    }
  }

  bool containsDrawing(QString s) {
    return drawList.contains(s);
  }

 protected:
  void paintEvent(QPaintEvent *event);

 private:
  QMap<QString, QPixmap*> drawList;
};

#endif  // SRC_GUI_OVERLAY_H_
