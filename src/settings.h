#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>

class Settings : public QObject {
  Q_OBJECT
public:
  Settings(QObject *parent = 0);
  void write_settings();
  void read_settings();
};

#endif // SETTINGS_H
