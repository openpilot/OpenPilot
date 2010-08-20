TEMPLATE = lib
TARGET = Config
include(../../openpilotgcsplugin.pri)
include(../../plugins/coreplugin/coreplugin.pri)
include(../../plugins/uavobjects/uavobjects.pri)
OTHER_FILES += Config.pluginspec
HEADERS += configplugin.h \
    configgadgetconfiguration.h \
    configgadgetwidget.h \
    configgadgetfactory.h \
    configgadgetoptionspage.h \
    configgadget.h \
    fancytabwidget.h \
    configservowidget.h \
    configtaskwidget.h \
    configairframewidget.h \
    configtelemetrywidget.h
SOURCES += configplugin.cpp \
    configgadgetconfiguration.cpp \
    configgadgetwidget.cpp \
    configgadgetfactory.cpp \
    configgadgetoptionspage.cpp \
    configgadget.cpp \
    fancytabwidget.cpp \
    configtaskwidget.cpp \
    configservowidget.cpp \
    configairframewidget.cpp \
    configtelemetrywidget.cpp
FORMS += settingswidget.ui \
    airframe.ui \
    telemetry.ui
RESOURCES += configgadget.qrc
