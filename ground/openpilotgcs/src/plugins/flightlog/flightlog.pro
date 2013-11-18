TEMPLATE = lib 
TARGET = FlightLog

include(../../openpilotgcsplugin.pri)
include(../../plugins/coreplugin/coreplugin.pri) 

HEADERS += flightlogplugin.h
SOURCES += flightlogplugin.cpp

OTHER_FILES += Flightlog.pluginspec
