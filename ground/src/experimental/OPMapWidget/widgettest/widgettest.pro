DESTDIR = ../build
PROJECT = gettilestest
TEMPLATE = app
QT += network
QT += sql
QT += opengl
CONFIG += console
CONFIG -= app_bundle
DEPENDPATH += .
INCLUDEPATH += ../core
SOURCES += main.cpp \
    map.cpp
LIBS += -L../build \
    -lmapwidget# -lcore -linternals -lcore
HEADERS += map.h
