TEMPLATE = lib
TARGET = HITLIL2
QT += network
include(../../openpilotgcsplugin.pri)
include(hitlil2_dependencies.pri)
HEADERS += hitlil2plugin.h \
    hitlil2widget.h \
    hitlil2optionspage.h \
    hitlil2factory.h \
    hitlil2configuration.h \
    hitlil2.h \
    il2bridge.h
SOURCES += hitlil2plugin.cpp \
    hitlil2widget.cpp \
    hitlil2optionspage.cpp \
    hitlil2factory.cpp \
    hitlil2configuration.cpp \
    hitlil2.cpp \
    il2bridge.cpp
OTHER_FILES += HITLIL2.pluginspec
FORMS += hitlil2optionspage.ui \
    hitlil2widget.ui
