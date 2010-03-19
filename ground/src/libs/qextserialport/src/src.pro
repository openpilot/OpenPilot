TEMPLATE = lib
TARGET = QExtSerialPort
DEFINES += QEXTSERIALPORT_LIBRARY
include(../../../openpilotgcslibrary.pri)

#VERSION                 = 1.2.0

# event driven device enumeration on windows requires the gui module
!win32:QT               -= gui

HEADERS                 = qextserialport.h \
                          qextserialenumerator.h \
                          qextserialport_global.h
SOURCES                 = qextserialport.cpp

unix:SOURCES           += posix_qextserialport.cpp
unix:!macx:SOURCES     += qextserialenumerator_unix.cpp
macx {
  SOURCES          += qextserialenumerator_osx.cpp
  LIBS             += -framework IOKit -framework CoreFoundation
}

win32 {
  SOURCES          += win_qextserialport.cpp qextserialenumerator_win.cpp
  DEFINES          += WINVER=0x0501 # needed for mingw to pull in appropriate dbt business...probably a better way to do this
  LIBS             += -lsetupapi
}

