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
    uavobjectfieldprimitives.h \
    uavobjectfieldenum.h \
    uavobjectfieldstring.h \
    exampleobject2.h \
    exampleobject1.h
SOURCES += uavobject.cpp \
    uavmetaobject.cpp \
    uavobjectmanager.cpp \
    uavdataobject.cpp \
    uavobjectfield.cpp \
    uavobjectsinit.cpp \
    uavobjectsplugin.cpp \
    examplesettings.cpp \
    uavobjectfieldprimitives.cpp \
    uavobjectfieldenum.cpp \
    uavobjectfieldstring.cpp \
    exampleobject2.cpp \
    exampleobject1.cpp
DEFINES += UAVOBJECTS_LIBRARY
OTHER_FILES += UAVObjects.pluginspec
