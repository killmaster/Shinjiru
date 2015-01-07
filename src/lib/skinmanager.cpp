#include "skinmanager.h"

#include <QMutex>
#include <QApplication>

SkinManager* SkinManager::m_Instance = 0;

SkinManager* SkinManager::sharedManager() {
  static QMutex mutex;
  if (! m_Instance) {
    mutex.lock();

    if (! m_Instance) {
      m_Instance = new SkinManager;
    }

    mutex.unlock();
  }

  return m_Instance;
}

SkinManager::SkinManager() : QObject(0) {
  this->element_paths.insert(SkinElements::AboutMascot, "/data/skin/mascot.png");

}

SkinManager::~SkinManager() {

}

QString SkinManager::get(int id) {
  return QApplication::applicationDirPath() + this->element_paths.at(id);
}

