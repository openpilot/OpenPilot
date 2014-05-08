TEMPLATE = lib
TARGET = Telemetry

QT += svg

include(../../openpilotgcsplugin.pri) 
include(../../plugins/coreplugin/coreplugin.pri) 
include(../../libs/version_info/version_info.pri)
include(telemetry_dependencies.pri)

HEADERS += telemetry_global.h \
    telemetryplugin.h \
    monitorwidget.h \
    monitorgadgetconfiguration.h \
    monitorgadget.h \
    monitorgadgetfactory.h \
    monitorgadgetoptionspage.h

SOURCES += telemetryplugin.cpp \
    monitorwidget.cpp \
    monitorgadgetconfiguration.cpp \
    monitorgadget.cpp \
    monitorgadgetfactory.cpp \
    monitorgadgetoptionspage.cpp

DEFINES += TELEMETRY_LIBRARY

RESOURCES += telemetry.qrc

OTHER_FILES += Telemetry.pluginspec
