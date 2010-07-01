
TEMPLATE = lib 
TARGET = NotifyPlugin 
 
include(../../openpilotgcsplugin.pri) 
include(../../plugins/coreplugin/coreplugin.pri) 
include(notifyplugin_dependencies.pri)

QT        += phonon

HEADERS += notifyplugin.h \  
    notifypluginoptionspage.h \
    notifypluginconfiguration.h \
    notifyitemdelegate.h
SOURCES += notifyplugin.cpp \  
    notifypluginoptionspage.cpp \
    notifypluginconfiguration.cpp \
    notifyitemdelegate.cpp
 
OTHER_FILES += NotifyPlugin.pluginspec

FORMS += \
    notifypluginoptionspage.ui

RESOURCES += \
    res.qrc
