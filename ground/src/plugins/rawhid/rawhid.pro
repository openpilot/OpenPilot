TEMPLATE = lib
TARGET = RawHID
include(../../openpilotgcsplugin.pri)
include(rawhid_dependencies.pri)

HEADERS += rawhidplugin.h

SOURCES += rawhidplugin.cpp

FORMS += 

RESOURCES += 

DEFINES += RAWHID_LIBRARY
OTHER_FILES += RawHID.pluginspec
