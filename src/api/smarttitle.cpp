/* Copyright 2015 Kazakuri */

#include "./smarttitle.h"

SmartTitle::SmartTitle(QObject *parent) : QObject(parent) {
  useOffset = false;
  offset = 0;
  id = "0";
  title = "";
  custom = "";
}

SmartTitle::~SmartTitle() {
}


