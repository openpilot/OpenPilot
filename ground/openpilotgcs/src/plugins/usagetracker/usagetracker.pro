
TEMPLATE = lib
TARGET = UsageTracker

include(../../openpilotgcsplugin.pri)
include(../../libs/version_info/version_info.pri)
include(../../plugins/coreplugin/coreplugin.pri)
include(../../plugins/uavobjects/uavobjects.pri)
include(../../plugins/uavobjectutil/uavobjectutil.pri)
include(../../plugins/uavtalk/uavtalk.pri)

HEADERS += usagetrackerplugin.h
SOURCES += usagetrackerplugin.cpp

OTHER_FILES += usagetracker.pluginspec
QT += network
