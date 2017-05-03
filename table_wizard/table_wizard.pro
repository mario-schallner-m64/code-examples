#-------------------------------------------------
#
# Project created by QtCreator 2015-01-07T11:51:09
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = table_wizard
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    rb.cpp \
    rb_listwidgetitem.cpp \
    rb_displayer.cpp \
    rb_screen.cpp \
    rb_tools.cpp \
    rb_color_picker.cpp \
    rb_table_widget.cpp \
    rbsa_effect.cpp \
    scrn_effect.cpp \
    render_scrn.cpp \
    playlist_item_widget.cpp

HEADERS  += mainwindow.h \
    rb.h \
    rb_listwidgetitem.h \
    rb_displayer.h \
    rb_screen.h \
    rb_tools.h \
    rb_color_picker.h \
    rb_table_widget.h \
    rbsa_effect.h \
    scrn_effect.h \
    render_scrn.h \
    playlist_item_widget.h

FORMS    += mainwindow.ui

RESOURCES += \
    res/res.qrc
