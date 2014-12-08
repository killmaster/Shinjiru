QT       += core gui widgets network concurrent

TARGET = Shinjiru
TEMPLATE = app

CONFIG += c++11

CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

include(QtAwesome/QtAwesome/QtAwesome.pri)
include(AniListAPI/AniListAPI.pri)

SOURCES   += main.cpp\
             mainwindow.cpp \
    torrents.cpp \
    anitomy/anitomy/anitomy.cpp \
    anitomy/anitomy/element.cpp \
    anitomy/anitomy/keyword.cpp \
    anitomy/anitomy/parser.cpp \
    anitomy/anitomy/parser_helper.cpp \
    anitomy/anitomy/parser_number.cpp \
    anitomy/anitomy/string.cpp \
    anitomy/anitomy/token.cpp \
    anitomy/anitomy/tokenizer.cpp \
    filedownloader.cpp \
    anime.cpp \
    animepanel.cpp \
    airinganime.cpp \
    flowlayout.cpp \
    scrolltext.cpp

HEADERS   += mainwindow.h \
             app.h \
    torrents.h \
    anitomy/anitomy/anitomy.h \
    anitomy/anitomy/element.h \
    anitomy/anitomy/keyword.h \
    anitomy/anitomy/parser.h \
    anitomy/anitomy/string.h \
    anitomy/anitomy/token.h \
    anitomy/anitomy/tokenizer.h \
    filedownloader.h \
    anime.h \
    progresstablewidgetitem.h \
    animepanel.h \
    airinganime.h \
    flowlayout.h \
    scrolltext.h

FORMS     += mainwindow.ui \
    animepanel.ui \
    airinganime.ui

RESOURCES += res.qrc
