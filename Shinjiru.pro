QT += core gui widgets network concurrent

TARGET = Shinjiru
VERSION = 0.6.1
TEMPLATE = app

DEFINES += APP_VERSION=\\\"$$VERSION\\\"

CONFIG += c++11

win32:LIBS += -L"C:/Program Files (x86)/Microsoft SDKs/Windows/v7.1A/Lib" -lUser32 -lKernel32

win32-msvc* {
  QMAKE_CFLAGS_RELEASE = -O2 -MD -zi
  QMAKE_LFLAGS_RELEASE = /INCREMENTAL:NO /DEBUG
}

include(lib/QtAwesome/QtAwesome/QtAwesome.pri)
include(lib/AniListAPI/AniListAPI.pri)
include(lib/fervor/fervor.pri)

win32:include(src/lib/crashhandler/crash_handler.pri)
linux:include(src/lib/crashhandler/crash_handler.pri)

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
    src/gui/apiwebview.cpp \
    src/gui/rulemanager.cpp \
    src/gui/overlay.cpp \
    src/gui/searchpanel.cpp

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
    src/gui/facetablewidgetitem.h \
    src/gui/rulemanager.h \
    src/gui/overlay.h \
    src/gui/searchpanel.h

FORMS     += \
    src/gui/airinganime.ui \
    src/gui/animepanel.ui \
    src/gui/mainwindow.ui \
    src/gui/rulewizard.ui \
    src/gui/about.ui \
    src/gui/apiwebview.ui \
    src/gui/rulemanager.ui \
    src/gui/searchpanel.ui

RESOURCES += res/res.qrc

RC_FILE = Shinjiru.rc

macx: LIBS += -lssl

macx: LIBS += -lcrypto
