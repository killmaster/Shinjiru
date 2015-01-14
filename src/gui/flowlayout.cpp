#include "flowlayout.h"
#include "airinganime.h"

#include <QDebug>

FlowLayout::FlowLayout(QWidget *parent) : QLayout(parent) {
  m_hSpace = -1;
  m_vSpace = -1;
}

FlowLayout::~FlowLayout() {
  QLayoutItem *item;
  while ((item = takeAt(0)))
    delete item;
}

bool layoutItemLessThan(QLayoutItem* &v1, QLayoutItem* &v2) {
  QWidget *w1 = v1->widget();
  QWidget *w2 = v2->widget();
  AiringAnime *a1 = static_cast<AiringAnime *>(w1);
  AiringAnime *a2 = static_cast<AiringAnime *>(w2);
  return a1->getAnime()->getTitle() < a2->getAnime()->getTitle();
}

void FlowLayout::addItem(QLayoutItem *item) {
  itemList.append(item);

  qSort(itemList.begin(), itemList.end(), layoutItemLessThan);
}

int FlowLayout::horizontalSpacing() const {
  if (m_hSpace >= 0)
    return m_hSpace;
  else
    return smartSpacing(QStyle::PM_LayoutHorizontalSpacing);
}

int FlowLayout::verticalSpacing() const {
  if (m_vSpace >= 0)
    return m_vSpace;
  else
    return smartSpacing(QStyle::PM_LayoutVerticalSpacing);
}

int FlowLayout::count() const {
  return itemList.size();
}

QLayoutItem *FlowLayout::itemAt(int index) const {
  return itemList.value(index);
}

QLayoutItem *FlowLayout::takeAt(int index) {
  if (index >= 0 && index < itemList.size())
    return itemList.takeAt(index);
  else
    return 0;
}

Qt::Orientations FlowLayout::expandingDirections() const {
  return 0;
}

bool FlowLayout::hasHeightForWidth() const {
  return true;
}

int FlowLayout::contentsWidth() {
  if(itemList.count() == 0) return 0;

  int cwidth = itemList.at(0)->widget()->width();
  int spacing = this->smartSpacing(QStyle::PM_LayoutHorizontalSpacing);

  int temp = 0;

  while(true) {
    if(temp + cwidth >= this->geometry().width()) {
      temp -= spacing;
      break;
    }

    temp += cwidth + spacing;

    if(temp > this->geometry().width()) {
      temp -= spacing;
      break;
    }
  }

  return temp;
}

int FlowLayout::heightForWidth(int width) const {
  int height = doLayout(QRect(0, 0, width, 0), true);
  return height;
}

void FlowLayout::setGeometry(const QRect &rect) {
  QLayout::setGeometry(rect);
  doLayout(rect, false);
}

QSize FlowLayout::sizeHint() const {
  return minimumSize();
}

QSize FlowLayout::minimumSize() const {
  QSize size;
  QLayoutItem *item;
  foreach (item, itemList)
    size = size.expandedTo(item->minimumSize());

  size += QSize(2*margin(), 2*margin());
  return size;
}

int FlowLayout::doLayout(const QRect &rect, bool testOnly) const {
  int *left = new int;
  this->getContentsMargins(left, 0, 0, 0);
  QRect effectiveRect = rect.adjusted(+*left, 0, 0, 0);
  int x = effectiveRect.x();
  int y = effectiveRect.y();
  int lineHeight = 0;

  QLayoutItem *item;
  foreach (item, itemList) {
    QWidget *wid = item->widget();

    int spaceX = horizontalSpacing();
    if (spaceX == -1)
      spaceX = wid->style()->layoutSpacing(QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Horizontal);

    int spaceY = verticalSpacing();
    if (spaceY == -1)
      spaceY = wid->style()->layoutSpacing(QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Vertical);

    int nextX = x + item->sizeHint().width() + spaceX;
    if (nextX - spaceX > effectiveRect.right() && lineHeight > 0) {
      x = effectiveRect.x();
      y = y + lineHeight + spaceY;
      nextX = x + item->sizeHint().width() + spaceX;
      lineHeight = 0;

    }

    if (!testOnly)
      item->setGeometry(QRect(QPoint(x, y), item->sizeHint()));

    x = nextX;
    lineHeight = qMax(lineHeight, item->sizeHint().height());
  }

  return y + lineHeight - rect.y();
}

int FlowLayout::smartSpacing(QStyle::PixelMetric pm) const {
  if(pm == QStyle::PM_LayoutHorizontalSpacing && itemList.count() > 0 && this->geometry().width() > 0) {
    /*int ourWidth = this->geometry().width();
    int oneContentWidth = itemList.at(0)->widget()->width() + 10;
    if (oneContentWidth == 0) return 0;
    int numberOfContent = ourWidth / oneContentWidth;

    if(numberOfContent == 0) numberOfContent = 1;

    return (ourWidth - (oneContentWidth * numberOfContent)) / numberOfContent;*/


    return 25;
  }


  QObject *parent = this->parent();
  if (!parent)
    return -1;
  else if (parent->isWidgetType()) {
    QWidget *pw = static_cast<QWidget *>(parent);
    return pw->style()->pixelMetric(pm, 0, pw);
  } else
    return static_cast<QLayout *>(parent)->spacing();
}
