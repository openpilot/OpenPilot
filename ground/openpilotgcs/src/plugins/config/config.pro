TEMPLATE = lib
TARGET = Config
QT += svg

include(../../openpilotgcsplugin.pri)
include(../../libs/utils/utils.pri)
include(../../plugins/uavtalk/uavtalk.pri)
include(../../plugins/coreplugin/coreplugin.pri)
include(../../plugins/uavobjects/uavobjects.pri)
include(../../plugins/uavobjectutil/uavobjectutil.pri)

INCLUDEPATH += ../../libs/eigen

OTHER_FILES += Config.pluginspec

HEADERS += configplugin.h \
    configgadgetconfiguration.h \
    configgadgetwidget.h \
    configgadgetfactory.h \
    configgadgetoptionspage.h \
    configgadget.h \
    fancytabwidget.h \
    configinputwidget.h \
    configoutputwidget.h \
    configtaskwidget.h \
    configairframewidget.h \
    configtelemetrywidget.h \
    configahrswidget.h \
    configccattitudewidget.h \
    mixercurvewidget.h \
    mixercurvepoint.h \
    mixercurveline.h \
    configccpmwidget.h \
    configstabilizationwidget.h \
    assertions.h \
    calibration.h \
    defaultattitudewidget.h

SOURCES += configplugin.cpp \
    configgadgetconfiguration.cpp \
    configgadgetwidget.cpp \
    configgadgetfactory.cpp \
    configgadgetoptionspage.cpp \
    configgadget.cpp \
    fancytabwidget.cpp \
    configtaskwidget.cpp \
    configinputwidget.cpp \
    configoutputwidget.cpp \
    configairframewidget.cpp \
    configtelemetrywidget.cpp \
    configahrswidget.cpp \
    configccattitudewidget.cpp \
    mixercurvewidget.cpp \
    mixercurvepoint.cpp \
    mixercurveline.cpp \
    configccpmwidget.cpp \
    configstabilizationwidget.cpp \
    twostep.cpp \
    legacy-calibration.cpp \
    gyro-calibration.cpp \
    alignment-calibration.cpp \
    defaultattitudewidget.cpp
    
FORMS +=  \
    airframe.ui \
    telemetry.ui \
    ahrs.ui \
    ccpm.ui \
    stabilization.ui \
    input.ui \
    output.ui \
    ccattitude.ui \
    defaultattitude.ui

RESOURCES += configgadget.qrc
