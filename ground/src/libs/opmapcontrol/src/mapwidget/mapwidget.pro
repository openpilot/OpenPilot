TEMPLATE = lib
TARGET = OPMapWidget
DEFINES += OPMAPWIDGET_LIBRARY

include(../../../../openpilotgcslibrary.pri)

SOURCES += opmapcontrol.cpp \
    mapgraphicitem.cpp \
    opmapwidget.cpp

LIBS += -L../build -lcore -linternals -lcore
HEADERS += opmapcontrol.h \
    mapgraphicitem.h \
    opmapwidget.h 

QT += opengl
QT += network
QT += sql
