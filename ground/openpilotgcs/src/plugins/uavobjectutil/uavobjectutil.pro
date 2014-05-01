TEMPLATE = lib
TARGET = UAVObjectUtil
DEFINES += UAVOBJECTUTIL_LIBRARY
include(../../openpilotgcsplugin.pri)
include(uavobjectutil_dependencies.pri)

HEADERS += uavobjectutil_global.h \
	uavobjectutilmanager.h \
    uavobjectutilplugin.h \
   devicedescriptorstruct.h \
    uavobjecthelper.h

SOURCES += uavobjectutilmanager.cpp \
    uavobjectutilplugin.cpp \
    uavobjecthelper.cpp

OTHER_FILES += UAVObjectUtil.pluginspec
