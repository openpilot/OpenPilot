TEMPLATE = lib 
TARGET = MapGadget

include(../../openpilotgcsplugin.pri)
include(../../plugins/coreplugin/coreplugin.pri) 

include(../../libs/qmapcontrol/qmapcontrol.pri)

HEADERS += mapplugin.h 
HEADERS += mapgadget.h 
HEADERS += mapgadgetwidget.h 
HEADERS += mapgadgetfactory.h 
SOURCES += mapplugin.cpp 
SOURCES += mapgadget.cpp 
SOURCES += mapgadgetfactory.cpp
SOURCES += mapgadgetwidget.cpp

OTHER_FILES += MapGadget.pluginspec
