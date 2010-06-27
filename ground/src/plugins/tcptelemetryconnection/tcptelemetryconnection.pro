TEMPLATE = lib
TARGET = TCPtelemetry
include(../../openpilotgcsplugin.pri)
include(tcptelemetry_dependencies.pri)
HEADERS += tcptelemetryplugin.h \
    tcptelemetry_global.h \
    tcptelemetryconfiguration.h \
    tcptelemetryoptionspage.h
SOURCES += tcptelemetryplugin.cpp \
    tcptelemetryconfiguration.cpp \
    tcptelemetryoptionspage.cpp
FORMS += tcptelemetryoptionspage.ui
RESOURCES += 
DEFINES += TCPtelemetry_LIBRARY
OTHER_FILES += TCPtelemetry.pluginspec
QT += network
