TARGET = finaltest
TEMPLATE = app

#include(../../opmapcontrol.pri)
LIBS+=../../../../../lib/openpilotgcs/OPMapWidgetd.dll
DESTDIR = ../../../../../bin


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui
