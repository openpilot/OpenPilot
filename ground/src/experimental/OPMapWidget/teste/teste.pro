DESTDIR                 = ../build
PROJECT = teste
TEMPLATE = app
QT += network
QT += sql
CONFIG += console
CONFIG -= app_bundle
DEPENDPATH += .
INCLUDEPATH  += ../core
SOURCES += main.cpp

LIBS += -L../build -lcore -linternals
