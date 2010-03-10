TEMPLATE = lib
TARGET = UAVObjects

include(../../openpilotgcsplugin.pri)
include(uavobjects_dependencies.pri)

HEADERS += uavobject.h \
    uavmetaobject.h \
    uavobjectmanager.h \
    uavdataobject.h \
    uavobjectfield.h \
    uavobjectsinit.h \
    uavobjectsplugin.h

SOURCES += uavobject.cpp \
    uavmetaobject.cpp \
    uavobjectmanager.cpp \
    uavdataobject.cpp \
    uavobjectfield.cpp \
    uavobjectsinit.cpp \
    uavobjectsplugin.cpp

OTHER_FILES += UAVObjects.pluginspec
