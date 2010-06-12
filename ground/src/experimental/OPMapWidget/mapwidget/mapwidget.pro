include (../common.pri)

QT += opengl
CONFIG -= staticlib

SOURCES += opmapcontrol.cpp \
    mapgraphicitem.cpp \
    opmapwidget.cpp

LIBS += -L../build -lcore -linternals -lcore
HEADERS += opmapcontrol.h \
    mapgraphicitem.h \
    opmapwidget.h 


