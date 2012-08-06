TEMPLATE = lib
TARGET = HITLNEW
QT += network
include(../../openpilotgcsplugin.pri)
include(hitlnew_dependencies.pri)
HEADERS += hitlplugin.h \
    hitlwidget.h \
    hitloptionspage.h \
    hitlfactory.h \
    hitlconfiguration.h \
    hitlgadget.h \
    hitlnoisegeneration.h \
    simulator.h \
    aerosimrcsimulator.h \
    fgsimulator.h \
    il2simulator.h \
    xplanesimulator.h
SOURCES += hitlplugin.cpp \
    hitlwidget.cpp \
    hitloptionspage.cpp \
    hitlfactory.cpp \
    hitlconfiguration.cpp \
    hitlgadget.cpp \
    hitlnoisegeneration.cpp \
    simulator.cpp \
    aerosimrcsimulator.cpp \
    fgsimulator.cpp \
    il2simulator.cpp \
    xplanesimulator.cpp
OTHER_FILES += hitlnew.pluginspec
FORMS += hitloptionspage.ui \
    hitlwidget.ui
RESOURCES += hitlresources.qrc
