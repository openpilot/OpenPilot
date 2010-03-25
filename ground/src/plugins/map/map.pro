TEMPLATE = lib
TARGET = MapGadget
include(../../openpilotgcsplugin.pri)
include(../../plugins/coreplugin/coreplugin.pri)
include(../../libs/qmapcontrol/qmapcontrol.pri)
HEADERS += mapplugin.h \
    mapgadgetconfiguration.h \
    mapgadget.h \
    mapgadgetwidget.h \
    mapgadgetfactory.h \
    mapgadgetoptionspage.h
SOURCES += mapplugin.cpp \
    mapgadgetconfiguration.cpp \
    mapgadget.cpp \
    mapgadgetfactory.cpp \
    mapgadgetwidget.cpp \
    mapgadgetoptionspage.cpp
OTHER_FILES += MapGadget.pluginspec
