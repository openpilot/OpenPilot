TEMPLATE = lib
TARGET = AirspeedGadget
QT += svg
include(../../openpilotgcsplugin.pri)
include(../../plugins/coreplugin/coreplugin.pri)
include(airspeed_dependencies.pri)
HEADERS += airspeedplugin.h
HEADERS += airspeedgadget.h
HEADERS += airspeedgadgetwidget.h
HEADERS += airspeedgadgetfactory.h
HEADERS += airspeedgadgetconfiguration.h
HEADERS += airspeedgadgetoptionspage.h
SOURCES += airspeedplugin.cpp
SOURCES += airspeedgadget.cpp
SOURCES += airspeedgadgetfactory.cpp
SOURCES += airspeedgadgetwidget.cpp
SOURCES += airspeedgadgetconfiguration.cpp
SOURCES += airspeedgadgetoptionspage.cpp
OTHER_FILES += AirspeedGadget.pluginspec
FORMS += airspeedgadgetoptionspage.ui
