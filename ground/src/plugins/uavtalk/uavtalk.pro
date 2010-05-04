TEMPLATE = lib
TARGET = UAVTalk
include(../../openpilotgcsplugin.pri)
include(uavtalk_dependencies.pri)
HEADERS += uavtalk.h \
    uavtalkplugin.h \
    telemetrymonitor.h
SOURCES += uavtalk.cpp \
    uavtalkplugin.cpp \
    telemetrymonitor.cpp
HEADERS += telemetry.h
SOURCES += telemetry.cpp
OTHER_FILES += UAVTalk.pluginspec
