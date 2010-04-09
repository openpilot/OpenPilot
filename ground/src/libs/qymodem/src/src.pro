TEMPLATE = lib
TARGET = QYmodem
DEFINES += QYMODEM_LIBRARY
include(../../../openpilotgcslibrary.pri)

# CONFIG                 += staticlib
SOURCES += qymodem.cpp \
    qymodem_tx.cpp \
    qymodemsend.cpp
HEADERS += qymodem_tx.h \
    qymodem.h \
    qymodemsend.h

LIBS += -l$$qtLibraryTarget(QExtSerialPort)

#CONFIG(debug, debug|release):LIBS += -lqextserialportd
#else:LIBS += -lqextserialport
win32:LIBS += -lsetupapi
