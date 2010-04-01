TEMPLATE = lib
TARGET = UAVObjects
include(../../openpilotgcsplugin.pri)
include(uavobjects_dependencies.pri)
HEADERS += uavobjects_global.h \
    uavobject.h \
    uavmetaobject.h \
    uavobjectmanager.h \
    uavdataobject.h \
    uavobjectfield.h \
    uavobjectsinit.h \
    uavobjectsplugin.h \
    examplesettings.h \
    exampleobject.h \
    uavobjectfieldprimitives.h \
    uavobjectfieldenum.h \
    uavobjectfieldstring.h
SOURCES += uavobject.cpp \
    uavmetaobject.cpp \
    uavobjectmanager.cpp \
    uavdataobject.cpp \
    uavobjectfield.cpp \
    uavobjectsinit.cpp \
    uavobjectsplugin.cpp \
    examplesettings.cpp \
    exampleobject.cpp \
    uavobjectfieldprimitives.cpp \
    uavobjectfieldenum.cpp \
    uavobjectfieldstring.cpp
DEFINES += UAVOBJECTS_LIBRARY
OTHER_FILES += UAVObjects.pluginspec
