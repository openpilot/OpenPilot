include(../../../openpilotgcslibrary.pri)
PROJECT = qymodem
TEMPLATE = lib
DEFINES += QYMODEM_LIBRARY
TARGET = Qymodem
# CONFIG                 += staticlib
SOURCES += qymodem.cpp \
    qymodem_TX.cpp \
    qymodemsend.cpp
HEADERS += qymodem_TX.h \
    qymodem.h \
    qymodemsend.h



CONFIG(debug, debug|release):LIBS += -lqextserialportd
else:LIBS += -lqextserialport
win32:LIBS += -lsetupapi

