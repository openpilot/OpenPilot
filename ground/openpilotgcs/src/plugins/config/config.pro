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
    config_pro_hw_widget.h \
    config_cc_hw_widget.h \
    configahrswidget.h \
    configccattitudewidget.h \
    mixercurvewidget.h \
    mixercurvepoint.h \
    mixercurveline.h \
    configccpmwidget.h \
    configstabilizationwidget.h \
    assertions.h \
    calibration.h \
    defaultattitudewidget.h \
    smartsavebutton.h \
    defaulthwsettingswidget.h \
    inputchannelform.h

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
    config_pro_hw_widget.cpp \
    config_cc_hw_widget.cpp \
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
    defaultattitudewidget.cpp \
    smartsavebutton.cpp \
    defaulthwsettingswidget.cpp \
    inputchannelform.cpp
    
FORMS +=  \
    airframe.ui \
    cc_hw_settings.ui \
    pro_hw_settings.ui \
    ahrs.ui \
    ccpm.ui \
    stabilization.ui \
    input.ui \
    output.ui \
    ccattitude.ui \
    defaultattitude.ui \
    defaulthwsettings.ui \
    inputchannelform.ui

RESOURCES += configgadget.qrc
