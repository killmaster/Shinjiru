#ifndef OVERLAY_H
#define OVERLAY_H

#include <QWidget>
#include <QPixmap>
#include <QMap>

class Overlay : public QWidget {
public:
  Overlay(QWidget *parent);
  void addDrawing(QString s, QPixmap *p) {
    drawList.insert(s, p);
    update();
  }
  void removeDrawing(QString s) {
    if(drawList.contains(s))
      delete drawList.take(s);

    update();
  }

  bool containsDrawing(QString s) {
    return drawList.contains(s);
  }

protected:
  void paintEvent(QPaintEvent *event);

private:
  QMap<QString, QPixmap*> drawList;
};

#endif
