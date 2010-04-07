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
    uavobjectfieldenum.h \
    uavobjectfieldstring.h \
    exampleobject2.h \
    exampleobject1.h \
    uavobjectfieldint8.h \
    uavobjectfieldint16.h \
    uavobjectfieldint32.h \
    uavobjectfieldfloat.h \
    uavobjectfielduint8.h \
    uavobjectfielduint16.h \
    uavobjectfielduint32.h \
    uavobjectfields.h
SOURCES += uavobject.cpp \
    uavmetaobject.cpp \
    uavobjectmanager.cpp \
    uavdataobject.cpp \
    uavobjectfield.cpp \
    uavobjectsinit.cpp \
    uavobjectsplugin.cpp \
    examplesettings.cpp \
    uavobjectfieldenum.cpp \
    uavobjectfieldstring.cpp \
    exampleobject2.cpp \
    exampleobject1.cpp \
    uavobjectfieldint8.cpp \
    uavobjectfieldint16.cpp \
    uavobjectfieldint32.cpp \
    uavobjectfieldfloat.cpp \
    uavobjectfielduint8.cpp \
    uavobjectfielduint16.cpp \
    uavobjectfielduint32.cpp
DEFINES += UAVOBJECTS_LIBRARY
OTHER_FILES += UAVObjects.pluginspec
