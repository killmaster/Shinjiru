/* Copyright 2015 Kazakuri */

#ifndef PROGRESSTABLEWIDGETITEM_H
#define PROGRESSTABLEWIDGETITEM_H

#include <QTableWidgetItem>

class ProgressTableWidgetItem : public QTableWidgetItem
{
public:
  bool operator <(const QTableWidgetItem & other) const {
    return text().split(" ").at(0).toInt() < other.text().split(" ").at(0).toInt();
  }
};

#endif // PROGRESSTABLEWIDGETITEM_H
