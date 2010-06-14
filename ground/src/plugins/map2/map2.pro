TEMPLATE = lib
TARGET = Map2Gadget
include(../../openpilotgcsplugin.pri)
include(../../plugins/coreplugin/coreplugin.pri)
include(../../libs/opmapcontrol/opmapcontrol.pri)
include(../../plugins/uavobjects/uavobjects.pri)
HEADERS += map2plugin.h
SOURCES += map2plugin.cpp
OTHER_FILES += Map2Gadget.pluginspec
FORMS += map2gadgetoptionspage.ui
RESOURCES += map2.qrc
