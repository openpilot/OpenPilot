#-------------------------------------------------
#
# Project created by QtCreator 2010-08-25T23:31:09
#
#-------------------------------------------------

QT       += core
QT       -= gui

TARGET = SerialLogger
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

INCLUDEPATH += ../../libs/qextserialport/src

# event driven device enumeration on windows requires the gui module
win32:QT               += gui

HEADERS                 = ../../libs/qextserialport/src/qextserialport.h \
                          ../../libs/qextserialport/src/qextserialenumerator.h \
                          ../../libs/qextserialport/src/qextserialport_global.h
SOURCES                 = ../../libs/qextserialport/src/qextserialport.cpp

unix:SOURCES           += ../../libs/qextserialport/src/posix_qextserialport.cpp
unix:!macx:SOURCES     += ../../libs/qextserialport/src/qextserialenumerator_unix.cpp
macx {
  SOURCES          += ../../libs/qextserialport/src/qextserialenumerator_osx.cpp
  LIBS             += -framework IOKit -framework CoreFoundation
}

win32 {
  SOURCES          += ../../libs/qextserialport/src/win_qextserialport.cpp
  SOURCES          += ../../libs/qextserialport/src/qextserialenumerator_win.cpp
  DEFINES          += WINVER=0x0501 # needed for mingw to pull in appropriate dbt business...probably a better way to do this
  LIBS             += -lsetupapi
}

SOURCES += main.cpp
