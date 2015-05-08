/* Copyright 2015 Kazakuri */

#include "./skinmanager.h"

#include <QMutex>
#include <QApplication>

SkinManager* SkinManager::m_Instance = 0;

SkinManager* SkinManager::sharedManager() {
  static QMutex mutex;
  if (!m_Instance) {
    mutex.lock();

    if (!m_Instance) {
      m_Instance = new SkinManager;
    }

    mutex.unlock();
  }

  return m_Instance;
}

SkinManager::SkinManager() : QObject(0) {
  this->element_paths.insert(SkinElements::AboutMascot,
                             "/data/skin/mascot.png");
  this->element_paths.insert(SkinElements::AdvancedIcon,
                             "/data/skin/advanced.png");
  this->element_paths.insert(SkinElements::AnimeListIcon,
                             "/data/skin/animelist.png");
  this->element_paths.insert(SkinElements::ApplicationIcon,
                             "/data/skin/application.png");
  this->element_paths.insert(SkinElements::RecognitionIcon,
                             "/data/skin/recognition.png");
  this->element_paths.insert(SkinElements::TorrentsIcon,
                             "/data/skin/torrents.png");
}

SkinManager::~SkinManager() {
}

QString SkinManager::get(int id) {
  return QApplication::applicationDirPath() + this->element_paths.value(id);
}

