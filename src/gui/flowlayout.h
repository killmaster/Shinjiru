/* Copyright 2015 Kazakuri */

#ifndef SRC_GUI_FLOWLAYOUT_H_
#define SRC_GUI_FLOWLAYOUT_H_

#include <QLayout>
#include <QRect>
#include <QWidgetItem>
#include <QStyle>
#include <QWidget>

class FlowLayout : public QLayout {
 public:
  explicit FlowLayout(QWidget *parent);
  ~FlowLayout();

  void addItem(QLayoutItem *item);
  int horizontalSpacing() const;
  int verticalSpacing() const;
  Qt::Orientations expandingDirections() const;
  bool hasHeightForWidth() const;
  int heightForWidth(int width) const;
  int count() const;
  QLayoutItem *itemAt(int index) const;
  QSize minimumSize() const;
  void setGeometry(const QRect &rect);
  QSize sizeHint() const;
  QLayoutItem *takeAt(int index);
  int contentsWidth();

  void disableSort() {
    sort = false;
  }

 private:
  int doLayout(const QRect &rect, bool testOnly) const;
  int smartSpacing(QStyle::PixelMetric pm) const;

  QList<QLayoutItem *> itemList;
  int m_hSpace;
  int m_vSpace;

  bool sort;
};

#endif  // SRC_GUI_FLOWLAYOUT_H_
