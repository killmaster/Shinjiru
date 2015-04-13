/* Copyright 2015 Kazakuri */

#ifndef FACETABLEWIDGETITEM_H
#define FACETABLEWIDGETITEM_H

#include <QTableWidgetItem>

class FaceTableWidgetItem : public QTableWidgetItem
{
public:
  bool operator <(const QTableWidgetItem & other) const {
    int i = text() == ":)" ? 3 : text() == ":|" ? 2 : 1;
    int j = other.text() == ":)" ? 3 : other.text() == ":|" ? 2 : 1;
    return i < j;
  }
};

#endif // FACETABLEWIDGETITEM_H
