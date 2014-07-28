TEMPLATE = lib
TARGET = GCSControl

QT += svg opengl network

include(../../openpilotgcsplugin.pri)
include(../../plugins/coreplugin/coreplugin.pri)
include(../../plugins/uavobjects/uavobjects.pri)
include(../../libs/sdlgamepad/sdlgamepad.pri)

HEADERS += \
    gcscontrolgadget.h \
    gcscontrolgadgetconfiguration.h \
    gcscontrolgadgetoptionspage.h \
    gcscontrolgadgetwidget.h \
    gcscontrolgadgetfactory.h \
    gcscontrolplugin.h \
    joystickcontrol.h

SOURCES += \
    gcscontrolgadget.cpp \
    gcscontrolgadgetconfiguration.cpp \
    gcscontrolgadgetoptionspage.cpp \
    gcscontrolgadgetwidget.cpp \
    gcscontrolgadgetfactory.cpp \
    gcscontrolplugin.cpp \
    joystickcontrol.cpp

OTHER_FILES += GCSControl.pluginspec

FORMS += \
    gcscontrol.ui \
    gcscontrolgadgetoptionspage.ui

RESOURCES += gcscontrol.qrc
