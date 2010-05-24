DESTDIR                 = ../build
QT += network
QT += sql
INCLUDEPATH += . ..
WARNINGS += -Wall
#CONFIG += console
#CONFIG -= app_bundle
CONFIG += dll
TEMPLATE = lib


UI_DIR = uics
MOC_DIR = mocs
OBJECTS_DIR = objs

