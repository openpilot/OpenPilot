include (../common.pri)
SOURCES += opmapcontrol.cpp
LIBS += -L../build \
    -lcore \
    -linternals
HEADERS += opmapcontrol.h
