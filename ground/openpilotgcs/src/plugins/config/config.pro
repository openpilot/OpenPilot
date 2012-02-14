TEMPLATE = lib
TARGET = Config
DEFINES += CONFIG_LIBRARY
QT += svg
include(config_dependencies.pri)
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
    configairframewidget.h \
    config_pro_hw_widget.h \
    config_cc_hw_widget.h \
    configahrswidget.h \
    configccattitudewidget.h \
    configccpmwidget.h \
    configstabilizationwidget.h \
    assertions.h \
    calibration.h \
    defaultattitudewidget.h \
    defaulthwsettingswidget.h \
    inputchannelform.h \
    configcamerastabilizationwidget.h \
    outputchannelform.h \
    config_global.h
SOURCES += configplugin.cpp \
    configgadgetconfiguration.cpp \
    configgadgetwidget.cpp \
    configgadgetfactory.cpp \
    configgadgetoptionspage.cpp \
    configgadget.cpp \
    fancytabwidget.cpp \
    configinputwidget.cpp \
    configoutputwidget.cpp \
    configairframewidget.cpp \
    config_pro_hw_widget.cpp \
    config_cc_hw_widget.cpp \
    configahrswidget.cpp \
    configccattitudewidget.cpp \
    configccpmwidget.cpp \
    configstabilizationwidget.cpp \
    twostep.cpp \
    legacy-calibration.cpp \
    gyro-calibration.cpp \
    alignment-calibration.cpp \
    defaultattitudewidget.cpp \
    defaulthwsettingswidget.cpp \
    inputchannelform.cpp \
    configcamerastabilizationwidget.cpp \
    outputchannelform.cpp
FORMS += airframe.ui \
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
    inputchannelform.ui \
    camerastabilization.ui \
    outputchannelform.ui
RESOURCES += configgadget.qrc
