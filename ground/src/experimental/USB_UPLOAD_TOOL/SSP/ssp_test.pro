#-------------------------------------------------
#
# Project created by QtCreator 2010-07-24T11:26:38
#
#-------------------------------------------------

QT       += core

QT       -= gui
DEFINES                 += QEXTSERIALPORT_LIBRARY
TARGET = ssp_test
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    qssp.cpp \
    port.cpp \
    qsspt.cpp
HEADERS                 +=  ../../../libs/qextserialport/src/qextserialport.h \
                           ../../../libs/qextserialport/src/qextserialenumerator.h \
                           ../../../libs/qextserialport/src/qextserialport_global.h \
    qssp.h \
    port.h \
    common.h \
    qsspt.h
SOURCES                 +=  ../../../libs/qextserialport/src/qextserialport.cpp

unix:SOURCES           += ../../../libs/qextserialport/src/posix_qextserialport.cpp
unix:!macx:SOURCES     += ../../../libs/qextserialport/src/qextserialenumerator_unix.cpp
macx {
  SOURCES          += ../../../libs/qextserialport/src/qextserialenumerator_osx.cpp
  LIBS             += -framework IOKit -framework CoreFoundation
}

win32 {
  SOURCES          += ../../../libs/qextserialport/src/win_qextserialport.cpp \
../../../libs/qextserialport/src/qextserialenumerator_win.cpp
  DEFINES          += WINVER=0x0501 # needed for mingw to pull in appropriate dbt business...probably a better way to do this
  LIBS             += -lsetupapi
}
