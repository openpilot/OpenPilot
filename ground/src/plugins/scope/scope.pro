TEMPLATE = lib 
TARGET = ScopeGadget

include(../../openpilotgcsplugin.pri)
include(../../plugins/coreplugin/coreplugin.pri) 
include(../../libs/qwt/qwt.pri)

HEADERS += scopeplugin.h
HEADERS += scopegadget.h
HEADERS += scopegadgetwidget.h
HEADERS += scopegadgetfactory.h
SOURCES += scopeplugin.cpp
SOURCES += scopegadget.cpp
SOURCES += scopegadgetfactory.cpp
SOURCES += scopegadgetwidget.cpp

OTHER_FILES += ScopeGadget.pluginspec
