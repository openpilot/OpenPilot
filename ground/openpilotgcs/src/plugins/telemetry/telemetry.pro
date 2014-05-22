TEMPLATE = lib
TARGET = Telemetry
DEFINES += TELEMETRY_LIBRARY

QT += svg

include(telemetry_dependencies.pri)
include(../../libs/version_info/version_info.pri)

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

OTHER_FILES += Telemetry.pluginspec

RESOURCES += telemetry.qrc
