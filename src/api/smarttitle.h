/* Copyright 2015 Kazakuri */

#ifndef SRC_API_SMARTTITLE_H_
#define SRC_API_SMARTTITLE_H_

#include <QObject>
#include <QFile>
#include <QApplication>
#include <QDir>
#include <QJsonObject>
#include <QJsonDocument>

class SmartTitle : public QObject {
  Q_OBJECT

 public:
  explicit SmartTitle(QObject *parent = 0);
  ~SmartTitle();

  void setID(QString id) {
    this->id = id;
  }

  void setOffset(int offset) {
    useOffset = true;
    this->offset = offset;
  }

  void setTitle(QString title) {
    this->title = title;
  }

  void setCustom(QString custom) {
    this->custom = custom;
  }

  bool hasOffset() {
    return useOffset;
  }

  int getOffset() {
    return offset;
  }

  bool contains(QString title) {
    return this->custom.toLower() == title.toLower();
  }

  QString getID() {
    return id;
  }

  QString getCustom() {
    return custom;
  }

  QString getTitle() {
    return title;
  }

 private:
  QString id;
  QString title;
  QString custom;

  bool useOffset;
  int offset;
};

#endif  // SRC_API_SMARTTITLE_H_
