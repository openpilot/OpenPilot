TEMPLATE = lib
TARGET = Config
QT += svg
include(../../openpilotgcsplugin.pri)
include(../../plugins/uavtalk/uavtalk.pri)
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
    configtelemetrywidget.h \
    configahrswidget.h \
    mixercurvewidget.h \
    mixercurvepoint.h \
    mixercurveline.h \
    configccpmwidget.h \
    configstabilizationwidget.h \
    vernierwidget.h
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
    configtelemetrywidget.cpp \
    configahrswidget.cpp \
    mixercurvewidget.cpp \
    mixercurvepoint.cpp \
    mixercurveline.cpp \
    configccpmwidget.cpp \
    configstabilizationwidget.cpp \
    vernierwidget.cpp
FORMS += settingswidget.ui \
    airframe.ui \
    telemetry.ui \
    ahrs.ui \
    ccpm.ui \
    stabilization.ui \
    vernier.ui
RESOURCES += configgadget.qrc
