TEMPLATE = lib
TARGET = QYModem
DEFINES += QYMODEM_LIBRARY

include(../../../openpilotgcslibrary.pri)
include(../../../libs/qextserialport/qextserialport.pri)

# CONFIG                 += staticlib
SOURCES += qymodem.cpp \
    qymodem_TX.cpp \
    qymodemsend.cpp
HEADERS += qymodem_TX.h \
    qymodem.h \
    qymodemsend.h

win32:LIBS += -lsetupapi

