/* Copyright 2015 Kazakuri */

#ifndef SRC_GUI_FACETABLEWIDGETITEM_H_
#define SRC_GUI_FACETABLEWIDGETITEM_H_

#include <QTableWidgetItem>

class FaceTableWidgetItem : public QTableWidgetItem {
 public:
  bool operator <(const QTableWidgetItem & other) const {
    int i = text() == ":)" ? 3 : text() == ":|" ? 2 : 1;
    int j = other.text() == ":)" ? 3 : other.text() == ":|" ? 2 : 1;
    return i < j;
  }
};

#endif  // SRC_GUI_FACETABLEWIDGETITEM_H_
