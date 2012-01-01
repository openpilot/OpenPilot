TEMPLATE = lib
TARGET = Config
QT += svg
include(../../openpilotgcsplugin.pri)
include(../../libs/utils/utils.pri)
include(../../plugins/uavtalk/uavtalk.pri)
include(../../plugins/coreplugin/coreplugin.pri)
include(../../plugins/uavobjects/uavobjects.pri)
include(../../plugins/uavobjectutil/uavobjectutil.pri)
include(../../plugins/uavsettingsimportexport/uavsettingsimportexport.pri)
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
    inputchannelform.h \
    configcamerastabilizationwidget.h \
    outputchannelform.h \
    configrevowidget.h
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
    inputchannelform.cpp \
    configcamerastabilizationwidget.cpp \
    outputchannelform.cpp \
    configrevowidget.cpp
FORMS += airframe.ui \
    cc_hw_settings.ui \
    pro_hw_settings.ui \
    ccpm.ui \
    stabilization.ui \
    input.ui \
    output.ui \
    ccattitude.ui \
    defaultattitude.ui \
    defaulthwsettings.ui \
    inputchannelform.ui \
    camerastabilization.ui \
    outputchannelform.ui \
    revosensors.ui
RESOURCES += configgadget.qrc
