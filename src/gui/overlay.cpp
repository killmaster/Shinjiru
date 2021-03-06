/* Copyright 2015 Kazakuri */

#include "./overlay.h"

#include <QPainter>
#include <QPen>

Overlay::Overlay(QWidget *parent) : QWidget(parent) {
  setPalette(Qt::transparent);
  setAttribute(Qt::WA_TransparentForMouseEvents);
}

void Overlay::paintEvent(QPaintEvent *event) {
  Q_UNUSED(event);

  QPainter painter(this);
  painter.setRenderHint(QPainter::HighQualityAntialiasing);

  for (QPixmap *p : drawList.values()) {
    painter.drawPixmap(0, 0, *p);
  }
}
