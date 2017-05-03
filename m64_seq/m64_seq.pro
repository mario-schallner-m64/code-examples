#-------------------------------------------------
#
# Project created by QtCreator 2016-05-16T14:20:38
#
#-------------------------------------------------

QT       += core gui
QT += serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = m64_seq
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    m64_qstepdisplay.cpp \
    m64_serial.cpp \
    m64_seq_step.cpp \
    m64_seq.cpp \
    m64_stepsequencer.cpp \
    form_edit_note.cpp \
    form_settings.cpp

HEADERS  += mainwindow.h \
    m64_qstepdisplay.h \
    m64_serial.h \
    m64_seq_step.h \
    m64_seq.h \
    m64_stepsequencer.h \
    form_edit_note.h \
    form_settings.h

FORMS    += mainwindow.ui \
    form_edit_note.ui \
    form_settings.ui

RESOURCES += \
    res.qrc
