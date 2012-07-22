include(../../../../../openpilotgcs.pri)

QT += core gui network

TEMPLATE = app
TARGET = udp_test
DESTDIR = $$GCS_APP_PATH

HEADERS += \
    udptestwidget.h

SOURCES += \
    udptestmain.cpp \
    udptestwidget.cpp

FORMS += \
    udptestwidget.ui
