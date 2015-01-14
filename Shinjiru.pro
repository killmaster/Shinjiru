QT += core gui widgets network concurrent

TARGET = Shinjiru
VERSION = 0.3.0
TEMPLATE = app

DEFINES += APP_VERSION=\\\"$$VERSION\\\"

CONFIG += c++11

LIBS += -L"C:/Program Files (x86)/Microsoft SDKs/Windows/v7.1A/Lib" -lUser32

include(lib/QtAwesome/QtAwesome/QtAwesome.pri)
include(lib/AniListAPI/AniListAPI.pri)
include(lib/fervor/fervor.pri)
include(src/lib/crashhandler/crash_handler.pri)

SOURCES   += \
    lib/anitomy/anitomy/anitomy.cpp \
    lib/anitomy/anitomy/element.cpp \
    lib/anitomy/anitomy/keyword.cpp \
    lib/anitomy/anitomy/parser.cpp \
    lib/anitomy/anitomy/parser_helper.cpp \
    lib/anitomy/anitomy/parser_number.cpp \
    lib/anitomy/anitomy/string.cpp \
    lib/anitomy/anitomy/token.cpp \
    lib/anitomy/anitomy/tokenizer.cpp \
    src/anitomy/anitomywrapper.cpp \
    src/api/anime.cpp \
    src/gui/airinganime.cpp \
    src/gui/animepanel.cpp \
    src/gui/flowlayout.cpp \
    src/gui/mainwindow.cpp \
    src/gui/scrolltext.cpp \
    src/lib/filedownloader.cpp \
    src/main.cpp \
    src/lib/torrentrss.cpp \
    src/api/user.cpp \
    src/api/api.cpp \
    src/settings.cpp \
    src/gui/userlisthelper.cpp \
    src/gui/userhelper.cpp \
    src/lib/windowwatcher.cpp \
    src/gui/rulewizard.cpp \
    src/gui/trayhelper.cpp \
    src/gui/torrentshelper.cpp \
    src/lib/skinmanager.cpp \
    src/gui/about.cpp \
    src/gui/settingshelper.cpp \
    src/gui/statshelper.cpp \
    src/gui/apiwebview.cpp

HEADERS   += \
    lib/anitomy/anitomy/anitomy.h \
    lib/anitomy/anitomy/element.h \
    lib/anitomy/anitomy/keyword.h \
    lib/anitomy/anitomy/parser.h \
    lib/anitomy/anitomy/string.h \
    lib/anitomy/anitomy/token.h \
    lib/anitomy/anitomy/tokenizer.h \
    src/anitomy/anitomywrapper.h \
    src/api/anime.h \
    src/gui/airinganime.h \
    src/gui/animepanel.h \
    src/gui/flowlayout.h \
    src/gui/mainwindow.h \
    src/gui/progresstablewidgetitem.h \
    src/gui/scrolltext.h \
    src/lib/filedownloader.h \
    src/app.h \
    src/lib/torrentrss.h \
    src/api/user.h \
    src/api/api.h \
    src/settings.h \
    src/lib/windowwatcher.h \
    src/gui/rulewizard.h \
    src/lib/skinmanager.h \
    src/gui/about.h \
    src/gui/apiwebview.h \
    src/gui/facetablewidgetitem.h

FORMS     += \
    src/gui/airinganime.ui \
    src/gui/animepanel.ui \
    src/gui/mainwindow.ui \
    src/gui/rulewizard.ui \
    src/gui/about.ui \
    src/gui/apiwebview.ui

RESOURCES += res/res.qrc

RC_FILE = Shinjiru.rc
