TEMPLATE = lib
TARGET = TCPtelemetry
include(../../openpilotgcsplugin.pri)
include(TCPtelemetry_dependencies.pri)
HEADERS += TCPtelemetryplugin.h \
    TCPtelemetry_global.h \
    TCPtelemetryconfiguration.h \
    TCPtelemetryoptionspage.h
SOURCES += TCPtelemetryplugin.cpp \
    TCPtelemetryconfiguration.cpp \
    TCPtelemetryoptionspage.cpp
FORMS += TCPtelemetryoptionspage.ui
RESOURCES += 
DEFINES += TCPtelemetry_LIBRARY
OTHER_FILES += TCPtelemetry.pluginspec
QT += network
