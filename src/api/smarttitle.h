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
  explicit SmartTitle(QObject *parent = 0, QString fileName = "");
  ~SmartTitle();

  void setID(QString id) {
    this->id = id;
  }

  void setOffset(int offset) {
    useOffset = true;
    this->offset = offset;
  }

  bool hasOffset() {
    return useOffset;
  }

  int getOffset() {
    return offset;
  }

  bool contains(QString title) {
    return this->source_title.toLower() == title.toLower();
  }

  QString getID() {
    return id;
  }

  void save() {
    QDir d(qApp->applicationDirPath() + "/relation");

    if(!d.exists())
      d.mkpath(".");

    if(file_name.isEmpty()) {
      file_name = qApp->applicationDirPath() + "/relation/" +
                  id + " - " + source_title + ".json";
    }

    QFile f(file_name);

    f.open(QFile::WriteOnly);

    QJsonObject json;

    json.insert("id", id);
    json.insert("title", source_title);

    if(useOffset)
      json.insert("offset", offset);

    f.write(QJsonDocument(json).toJson());
  }

  void load() {
    QFile f(qApp->applicationDirPath() + "/relation/" + file_name);

    f.open(QFile::ReadOnly);

    QByteArray data = f.readAll();
    QJsonObject json = QJsonDocument::fromJson(data).object();

    this->id = json.value("id").toString("0");
    this->source_title = json.value("title").toString();

    if(json.contains("offset"))
      this->setOffset(json.value("offset").toInt());
  }

 private:
  QString file_name;
  QString id;
  QString source_title;

  bool useOffset;
  int offset;

};

#endif  // SRC_API_SMARTTITLE_H_
