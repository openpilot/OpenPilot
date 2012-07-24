TEMPLATE = lib
TARGET = GCSControl 
QT += svg
QT += opengl
QT += network

include(../../openpilotgcsplugin.pri) 
include(../../plugins/coreplugin/coreplugin.pri) 
include(../../plugins/uavobjects/uavobjects.pri)
include(../../libs/sdlgamepad/sdlgamepad.pri)

HEADERS += gcscontrolgadget.h \
    gcscontrolgadgetconfiguration.h \
    gcscontrolgadgetoptionspage.h
HEADERS += joystickcontrol.h
HEADERS += gcscontrolgadgetwidget.h
HEADERS += gcscontrolgadgetfactory.h
HEADERS += gcscontrolplugin.h

SOURCES += gcscontrolgadget.cpp \
    gcscontrolgadgetconfiguration.cpp \
    gcscontrolgadgetoptionspage.cpp
SOURCES += gcscontrolgadgetwidget.cpp
SOURCES += gcscontrolgadgetfactory.cpp
SOURCES += gcscontrolplugin.cpp
SOURCES += joystickcontrol.cpp

OTHER_FILES += GCSControl.pluginspec

FORMS += gcscontrol.ui \
    gcscontrolgadgetoptionspage.ui

RESOURCES += gcscontrol.qrc
