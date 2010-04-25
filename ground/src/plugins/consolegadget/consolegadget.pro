TEMPLATE = lib
TARGET = ConsoleGadget
include(../../openpilotgcsplugin.pri)
include(../../plugins/coreplugin/coreplugin.pri)
HEADERS += consoleplugin.h \
    texteditloggerengine.h
HEADERS += consolegadget.h
HEADERS += consolegadgetwidget.h
HEADERS += consolegadgetfactory.h
SOURCES += consoleplugin.cpp \
    texteditloggerengine.cpp
SOURCES += consolegadget.cpp
SOURCES += consolegadgetfactory.cpp
SOURCES += consolegadgetwidget.cpp
OTHER_FILES += ConsoleGadget.pluginspec
