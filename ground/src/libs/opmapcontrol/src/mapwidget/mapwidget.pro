TEMPLATE = lib
TARGET = opmapwidget
DEFINES += OPMAPWIDGET_LIBRARY
include(../../../../openpilotgcslibrary.pri)

# DESTDIR = ../build
SOURCES += mapgraphicitem.cpp \
    opmapwidget.cpp \
    configuration.cpp \
    waypointitem.cpp \
    uavitem.cpp \
    trailitem.cpp \
    homeitem.cpp
LIBS += -L../build \
    -lcore \
    -linternals \
    -lcore
HEADERS += mapgraphicitem.h \
    opmapwidget.h \
    configuration.h \
    waypointitem.h \
    uavitem.h \
    uavmapfollowtype.h \
    uavtrailtype.h \
    trailitem.h \
    homeitem.h
QT += opengl
QT += network
QT += sql
QT += svg
RESOURCES += mapresources.qrc
