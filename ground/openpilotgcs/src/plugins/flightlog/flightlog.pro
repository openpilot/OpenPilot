TEMPLATE = lib 
TARGET = FlightLog

include(../../openpilotgcsplugin.pri)
include(../../plugins/coreplugin/coreplugin.pri) 

HEADERS += flightlogplugin.h \
    flightlogmanager.h
SOURCES += flightlogplugin.cpp \
    flightlogmanager.cpp

OTHER_FILES += Flightlog.pluginspec
