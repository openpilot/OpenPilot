TEMPLATE = lib
TARGET = HITL
QT += network
include(../../openpilotgcsplugin.pri)
include(hitl_dependencies.pri)
HEADERS += hitlplugin.h \
    hitlwidget.h \
    hitloptionspage.h \
    hitlfactory.h \
    hitlconfiguration.h \
    hitl.h \
    flightgearbridge.h
SOURCES += hitlplugin.cpp \
    hitlwidget.cpp \
    hitloptionspage.cpp \
    hitlfactory.cpp \
    hitlconfiguration.cpp \
    hitl.cpp \
    flightgearbridge.cpp
OTHER_FILES += HITL.pluginspec
FORMS += hitloptionspage.ui \
    hitlwidget.ui
RESOURCES += hitlresources.qrc
