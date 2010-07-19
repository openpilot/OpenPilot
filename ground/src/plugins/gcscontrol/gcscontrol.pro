TEMPLATE = lib 
TARGET = GCSControl 

include(../../openpilotgcsplugin.pri) 
include(../../plugins/coreplugin/coreplugin.pri) 
include(../../plugins/uavobjects/uavobjects.pri)

HEADERS += gcscontrolgadget.h
HEADERS += gcscontrolgadgetwidget.h
HEADERS += gcscontrolgadgetfactory.h
HEADERS += gcscontrolplugin.h

SOURCES += gcscontrolgadget.cpp
SOURCES += gcscontrolgadgetwidget.cpp
SOURCES += gcscontrolgadgetfactory.cpp
SOURCES += gcscontrolplugin.cpp

OTHER_FILES += GCSControl.pluginspec

FORMS += gcscontrol.ui
