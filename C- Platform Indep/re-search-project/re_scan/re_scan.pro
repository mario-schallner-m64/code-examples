#-------------------------------------------------
#
# Project created by QtCreator 2011-11-27T18:34:01
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = re_scan
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += \
    re_scan.cpp

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../re_core/release/ -lre_core
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../re_core/debug/ -lre_core
else:unix:!symbian: LIBS += -L$$OUT_PWD/../re_core/ -lre_core

INCLUDEPATH += $$PWD/../re_core
DEPENDPATH += $$PWD/../re_core

HEADERS +=
