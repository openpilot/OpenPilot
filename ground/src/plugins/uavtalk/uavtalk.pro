TEMPLATE = lib
TARGET = UAVTalk
include(../../openpilotgcsplugin.pri)
include(../../plugins/coreplugin/coreplugin.pri)
HEADERS += uavtalk.h \
    uavtalkplugin.h
SOURCES += uavtalk.cpp \
    uavtalkplugin.cpp
HEADERS += telemetry.h
SOURCES += telemetry.cpp
OTHER_FILES += uavtalk.pluginspec
