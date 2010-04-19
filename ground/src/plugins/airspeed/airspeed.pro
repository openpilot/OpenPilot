TEMPLATE = lib 
TARGET = AirspeedGadget
QT += svg

include(../../openpilotgcsplugin.pri)
include(../../plugins/coreplugin/coreplugin.pri) 
include(../../libs/qwt/qwt.pri)

HEADERS += airspeedplugin.h
HEADERS += airspeedgadget.h
HEADERS += airspeedgadgetwidget.h
HEADERS += airspeedgadgetfactory.h
SOURCES += airspeedplugin.cpp
SOURCES += airspeedgadget.cpp
SOURCES += airspeedgadgetfactory.cpp
SOURCES += airspeedgadgetwidget.cpp

OTHER_FILES += AirspeedGadget.pluginspec
