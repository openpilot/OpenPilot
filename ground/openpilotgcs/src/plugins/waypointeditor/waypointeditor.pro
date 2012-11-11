TEMPLATE = lib
TARGET = WaypointEditor 

include(../../openpilotgcsplugin.pri) 
include(../../plugins/coreplugin/coreplugin.pri) 
include(../../plugins/uavobjects/uavobjects.pri)

HEADERS += waypointeditorgadget.h \
    waypointtable.h
HEADERS += waypointeditorgadgetwidget.h
HEADERS += waypointeditorgadgetfactory.h
HEADERS += waypointeditorplugin.h

SOURCES += waypointeditorgadget.cpp \
    waypointtable.cpp
SOURCES += waypointeditorgadgetwidget.cpp
SOURCES += waypointeditorgadgetfactory.cpp
SOURCES += waypointeditorplugin.cpp

OTHER_FILES += waypointeditor.pluginspec

FORMS += waypointeditor.ui

RESOURCES += waypointeditor.qrc


