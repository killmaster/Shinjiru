QT       += core gui widgets network

TARGET = Shinjiru
TEMPLATE = app

CONFIG += c++11

include(QtAwesome/QtAwesome.pri)

SOURCES   += main.cpp\
             mainwindow.cpp \
    anilist.cpp \
    torrents.cpp \
    anitomy/anitomy/anitomy.cpp \
    anitomy/anitomy/element.cpp \
    anitomy/anitomy/keyword.cpp \
    anitomy/anitomy/parser.cpp \
    anitomy/anitomy/parser_helper.cpp \
    anitomy/anitomy/parser_number.cpp \
    anitomy/anitomy/string.cpp \
    anitomy/anitomy/token.cpp \
    anitomy/anitomy/tokenizer.cpp

HEADERS   += mainwindow.h \
             app.h \
    anilist.h \
    torrents.h \
    anitomy/anitomy/anitomy.h \
    anitomy/anitomy/element.h \
    anitomy/anitomy/keyword.h \
    anitomy/anitomy/parser.h \
    anitomy/anitomy/string.h \
    anitomy/anitomy/token.h \
    anitomy/anitomy/tokenizer.h

FORMS     += mainwindow.ui

RESOURCES += res.qrc
