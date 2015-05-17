QT += core gui widgets network concurrent

TARGET = Shinjiru
VERSION = 1.7.2
TEMPLATE = app
ICON = Shinjiru.icns

DEFINES += APP_VERSION=\\\"$$VERSION\\\"

CONFIG += c++11

win32:LIBS += -L"C:/Program Files (x86)/Microsoft SDKs/Windows/v7.1A/Lib" -lUser32 -lKernel32

win32-msvc* {
  QMAKE_CXXFLAGS_RELEASE = -O2 -MD
  QMAKE_LFLAGS_RELEASE = /INCREMENTAL:NO /DEBUG /MAP /OPT:REF
}

include(lib/QtAwesome/QtAwesome/QtAwesome.pri)
include(lib/AniListAPI/AniListAPI.pri)

include(lib/fervor/Fervor.pri)
CONFIG += fervor_gui
DEFINES += FV_GUI

win32:include(src/lib/crashhandler/crash_handler.pri)

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
    src/gui/searchpanel.cpp \
    src/gui/browserhelper.cpp \
    src/gui/browseanime.cpp \
    src/api/smarttitle.cpp \
    src/gui/settingsdialog.cpp

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
    src/gui/searchpanel.h \
    src/version.h \
    src/gui/browseanime.h \
    src/api/smarttitle.h \
    src/gui/settingsdialog.h

FORMS     += \
    src/gui/airinganime.ui \
    src/gui/animepanel.ui \
    src/gui/mainwindow.ui \
    src/gui/rulewizard.ui \
    src/gui/about.ui \
    src/gui/apiwebview.ui \
    src/gui/rulemanager.ui \
    src/gui/searchpanel.ui \
    src/gui/browseanime.ui \
    src/gui/settingsdialog.ui

RESOURCES += res/res.qrc

TRANSLATIONS += shinjiru_en.ts

RC_FILE = Shinjiru.rc

macx: LIBS += -L../openssl_x86_64/
macx: LIBS += -lssl
macx: LIBS += -lcrypto
