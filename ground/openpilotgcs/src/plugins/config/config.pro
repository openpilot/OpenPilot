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
    configvehicletypewidget.h \
    config_pro_hw_widget.h \
    config_cc_hw_widget.h \
    configccattitudewidget.h \
    configpipxtremewidget.h \
    cfg_vehicletypes/configccpmwidget.h \
    configstabilizationwidget.h \
    assertions.h \
    calibration.h \
    defaultattitudewidget.h \
    defaulthwsettingswidget.h \
    inputchannelform.h \
    configcamerastabilizationwidget.h \
    configtxpidwidget.h \
    outputchannelform.h \    
    cfg_vehicletypes/configmultirotorwidget.h \
    cfg_vehicletypes/configgroundvehiclewidget.h \
    cfg_vehicletypes/configfixedwingwidget.h \
    cfg_vehicletypes/vehicleconfig.h \
    configrevowidget.h \
    config_global.h \
    mixercurve.h \
    dblspindelegate.h
SOURCES += configplugin.cpp \
    configgadgetconfiguration.cpp \
    configgadgetwidget.cpp \
    configgadgetfactory.cpp \
    configgadgetoptionspage.cpp \
    configgadget.cpp \
    fancytabwidget.cpp \
    configinputwidget.cpp \
    configoutputwidget.cpp \
    configvehicletypewidget.cpp \
    config_pro_hw_widget.cpp \
    config_cc_hw_widget.cpp \
    configccattitudewidget.cpp \
    configstabilizationwidget.cpp \
    configpipxtremewidget.cpp \
    twostep.cpp \
    legacy-calibration.cpp \
    gyro-calibration.cpp \
    alignment-calibration.cpp \
    defaultattitudewidget.cpp \
    defaulthwsettingswidget.cpp \
    inputchannelform.cpp \
    configcamerastabilizationwidget.cpp \
    configrevowidget.cpp \
    configtxpidwidget.cpp \
    cfg_vehicletypes/configmultirotorwidget.cpp \
    cfg_vehicletypes/configgroundvehiclewidget.cpp \
    cfg_vehicletypes/configfixedwingwidget.cpp \
    cfg_vehicletypes/configccpmwidget.cpp \
    outputchannelform.cpp \
    cfg_vehicletypes/vehicleconfig.cpp \
    mixercurve.cpp \
    dblspindelegate.cpp
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
    revosensors.ui \
    txpid.ui \
    pipxtreme.ui \
    mixercurve.ui
RESOURCES += configgadget.qrc
