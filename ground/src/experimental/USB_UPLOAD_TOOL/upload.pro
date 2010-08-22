#-------------------------------------------------
#
# Project created by QtCreator 2010-07-24T11:26:38
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = HIDTest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    op_dfu.cpp
HEADERS += ../../plugins/rawhid/pjrc_rawhid.h \
    op_dfu.h

win32 {
    SOURCES += ../../plugins/rawhid/pjrc_rawhid_win.cpp
    LIBS += -lhid \
        -lsetupapi
}
