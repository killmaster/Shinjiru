/* Copyright 2015 Kazakuri */

#ifndef SRC_GUI_PROGRESSTABLEWIDGETITEM_H_
#define SRC_GUI_PROGRESSTABLEWIDGETITEM_H_

#include <QTableWidgetItem>

class ProgressTableWidgetItem : public QTableWidgetItem {
 public:
  bool operator <(const QTableWidgetItem & other) const {
    return text().split(" ").at(0).toInt() <
           other.text().split(" ").at(0).toInt();
  }
};

#endif  // SRC_GUI_PROGRESSTABLEWIDGETITEM_H_
