include (../common.pri)
SOURCES += opmapcontrol.cpp \
    mapgraphicitem.cpp \
    opmapwidget.cpp
LIBS += -L../build \
    -lcore \
    -linternals
HEADERS += opmapcontrol.h \
    mapgraphicitem.h \
    opmapwidget.h
