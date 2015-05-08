include(../../../../../openpilotgcs.pri)

QT += core gui network widgets

TEMPLATE = app
TARGET = udp_test

HEADERS += \
    udptestwidget.h

SOURCES += \
    udptestmain.cpp \
    udptestwidget.cpp

FORMS += \
    udptestwidget.ui
