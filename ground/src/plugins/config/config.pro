TEMPLATE = lib
TARGET = Config
include(../../openpilotgcsplugin.pri)
include(../../plugins/coreplugin/coreplugin.pri)
OTHER_FILES += Config.pluginspec
HEADERS += configplugin.h \
    configgadgetconfiguration.h \
    configgadgetwidget.h \
    configgadgetfactory.h \
    configgadgetoptionspage.h \
    configgadget.h
SOURCES += configplugin.cpp \
    configgadgetconfiguration.cpp \
    configgadgetwidget.cpp \
    configgadgetfactory.cpp \
    configgadgetoptionspage.cpp \
    configgadget.cpp
FORMS += configgadget.ui
