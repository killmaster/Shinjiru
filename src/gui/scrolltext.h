/* Copyright 2015 Kazakuri */

#ifndef SRC_GUI_SCROLLTEXT_H_
#define SRC_GUI_SCROLLTEXT_H_

#include <QWidget>
#include <QStaticText>
#include <QTimer>


class ScrollText : public QWidget {
  Q_OBJECT
  Q_PROPERTY(QString text READ text WRITE setText)
  Q_PROPERTY(QString separator READ separator WRITE setSeparator)

 public:
  explicit ScrollText(QWidget *parent = 0);

 public slots:  // NOLINT
  QString text() const;
  void setText(QString text);

  QString separator() const;
  void setSeparator(QString separator);


 protected:
  virtual void paintEvent(QPaintEvent *e);
  virtual void resizeEvent(QResizeEvent *e);

 private:
  void updateText();
  QString _text;
  QString _separator;
  QStaticText staticText;
  int singleTextWidth;
  QSize wholeTextSize;
  int leftMargin;
  bool scrollEnabled;
  int scrollPos;
  QImage alphaChannel;
  QImage buffer;
  QTimer timer;

 private slots:  // NOLINT
  virtual void timer_timeout();
};

#endif  // SRC_GUI_SCROLLTEXT_H_
