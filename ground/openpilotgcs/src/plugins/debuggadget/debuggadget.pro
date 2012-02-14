TEMPLATE = lib 
TARGET = DebugGadget

include(../../openpilotgcsplugin.pri)
include(../../plugins/coreplugin/coreplugin.pri)
HEADERS += debugplugin.h \
    debugengine.h
HEADERS += debuggadget.h
HEADERS += debuggadgetwidget.h
HEADERS += debuggadgetfactory.h
SOURCES += debugplugin.cpp \
    debugengine.cpp
SOURCES += debuggadget.cpp
SOURCES += debuggadgetfactory.cpp
SOURCES += debuggadgetwidget.cpp

OTHER_FILES += DebugGadget.pluginspec

FORMS += \
    debug.ui
