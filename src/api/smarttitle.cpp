/* Copyright 2015 Kazakuri */

#include "./smarttitle.h"

SmartTitle::SmartTitle(QObject *parent, QString fileName) : QObject(parent) {
  useOffset = false;
  offset = 0;
  id = "0";
  source_title = "";

  this->file_name = fileName;

  if (!file_name.isEmpty()) {
    load();
  }
}

SmartTitle::~SmartTitle() {
}


