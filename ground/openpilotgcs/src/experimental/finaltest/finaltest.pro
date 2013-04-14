#-------------------------------------------------
#
# Project created by QtCreator 2010-06-11T10:37:33
#
#-------------------------------------------------

TARGET = finaltest
TEMPLATE = app
DESTDIR = ../build
QT += network
QT += sql
QT += opengl
LIBS += -L../build \
    -lopmapwidgetd
SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui
