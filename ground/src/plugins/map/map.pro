TEMPLATE = lib
TARGET = MapGadget
include(../../openpilotgcsplugin.pri)
include(../../plugins/coreplugin/coreplugin.pri)
include(../../libs/qmapcontrol/qmapcontrol.pri)
HEADERS += mapplugin.h \
    mapgadgetconfiguration.h
HEADERS += mapgadget.h
HEADERS += mapgadgetwidget.h
HEADERS += mapgadgetfactory.h
HEADERS += mapgadgetoptionspage.h
SOURCES += mapplugin.cpp \
    mapgadgetconfiguration.cpp
SOURCES += mapgadget.cpp
SOURCES += mapgadgetfactory.cpp
SOURCES += mapgadgetwidget.cpp
SOURCES += mapgadgetoptionspage.cpp
OTHER_FILES += MapGadget.pluginspec
