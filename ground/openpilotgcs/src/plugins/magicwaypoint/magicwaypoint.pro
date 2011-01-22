TEMPLATE = lib
TARGET = MagicWaypoint 
QT += svg

include(../../openpilotgcsplugin.pri) 
include(../../plugins/coreplugin/coreplugin.pri) 
include(../../plugins/uavobjects/uavobjects.pri)

HEADERS += magicwaypointgadget.h
HEADERS += magicwaypointgadgetwidget.h
HEADERS += magicwaypointgadgetfactory.h
HEADERS += magicwaypointplugin.h
HEADERS += positionfield.h

SOURCES += magicwaypointgadget.cpp
SOURCES += magicwaypointgadgetwidget.cpp
SOURCES += magicwaypointgadgetfactory.cpp
SOURCES += magicwaypointplugin.cpp
SOURCES += positionfield.cpp

OTHER_FILES += MagicWaypoint.pluginspec

FORMS += magicwaypoint.ui

RESOURCES += magicwaypoint.qrc
