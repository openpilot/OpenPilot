TEMPLATE = lib
TARGET = HITLv2
QT += network

include(../../openpilotgcsplugin.pri)
include(hitlv2_dependencies.pri)

HEADERS += \
    aerosimrc.h \
    hitlv2configuration.h \
    hitlv2factory.h \
    hitlv2gadget.h \
    hitlv2optionspage.h \
    hitlv2plugin.h \
    hitlv2widget.h \
    simulatorv2.h

SOURCES += \
    aerosimrc.cpp \
    hitlv2configuration.cpp \
    hitlv2factory.cpp \
    hitlv2gadget.cpp \
    hitlv2optionspage.cpp \
    hitlv2plugin.cpp \
    hitlv2widget.cpp \
    simulatorv2.cpp

FORMS += \
    hitlv2optionspage.ui \
    hitlv2widget.ui

OTHER_FILES += hitlv2.pluginspec
