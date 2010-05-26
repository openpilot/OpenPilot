DESTDIR = ../build
PROJECT = gettilestest
TEMPLATE = app
QT += network
QT += sql
CONFIG += console
CONFIG -= app_bundle
DEPENDPATH += .
INCLUDEPATH += ../core
SOURCES += main.cpp \
    map.cpp
LIBS += -L../build \
    -lmapwidget -lcore -linternals
HEADERS += map.h
