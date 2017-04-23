#-------------------------------------------------
#
# Project created by QtCreator 2011-11-27T22:48:04
#
#-------------------------------------------------

QT       -= gui

TARGET = re_scriptlib
TEMPLATE = lib

DEFINES += RE_SCRIPTLIB_LIBRARY

SOURCES += re_scriptlib.cpp

HEADERS += re_scriptlib.h\
        re_scriptlib_global.h

symbian {
    #Symbian specific definitions
    MMP_RULES += EXPORTUNFROZEN
    TARGET.UID3 = 0xE6A3BFEA
    TARGET.CAPABILITY = 
    TARGET.EPOCALLOWDLLDATA = 1
    addFiles.sources = re_scriptlib.dll
    addFiles.path = !:/sys/bin
    DEPLOYMENT += addFiles
}

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/local/lib
    }
    INSTALLS += target
}

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../re_core/release/ -lre_core
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../re_core/debug/ -lre_core
else:unix:!symbian: LIBS += -L$$OUT_PWD/../re_core/ -lre_core

INCLUDEPATH += $$PWD/../re_core
DEPENDPATH += $$PWD/../re_core
