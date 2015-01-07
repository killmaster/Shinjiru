#ifndef SKINMANAGER_H
#define SKINMANAGER_H

#include <QObject>
#include <QStringList>

class SkinManager : public QObject
{
  Q_OBJECT

public:
  static SkinManager* sharedManager();
  QString get(int);

  enum SkinElements {
    AboutMascot
  };

signals:

public slots:

private:
  QStringList element_paths;
  static SkinManager* m_Instance;


  SkinManager();
  ~SkinManager();
  SkinManager(const SkinManager&);
  SkinManager& operator=(const SkinManager&);
};

#endif // SKINMANAGER_H
