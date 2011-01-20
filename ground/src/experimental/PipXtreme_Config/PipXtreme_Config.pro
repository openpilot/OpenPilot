#-------------------------------------------------
#
# Project created by QtCreator 2011-01-20T10:44:17
#
#-------------------------------------------------

QT       += core gui

TARGET = PipXtreme_Config
TEMPLATE = app

INCLUDEPATH += ../../libs/qextserialport/src

SOURCES += main.cpp\
        mainwindow.cpp \

HEADERS += mainwindow.h \

HEADERS                += ../../libs/qextserialport/src/qextserialport.h \
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
  SOURCES          += ../../libs/qextserialport/src/win_qextserialport.cpp \
                      ../../libs/qextserialport/src/qextserialenumerator_win.cpp
  DEFINES          += WINVER=0x0501 # needed for mingw to pull in appropriate dbt business...probably a better way to do this
  LIBS             += -lsetupapi
}

FORMS    += mainwindow.ui
