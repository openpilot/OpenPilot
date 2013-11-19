include_directories(../../libs/eigen)
include_directories(../uavobjectwidgetutils)

set(Config_SRCS
    configplugin.cpp
    configgadgetwidget.cpp
    configgadgetfactory.cpp
    configgadget.cpp
    fancytabwidget.cpp
    configinputwidget.cpp
    configoutputwidget.cpp
    configvehicletypewidget.cpp
    config_cc_hw_widget.cpp
    configccattitudewidget.cpp
    configstabilizationwidget.cpp
    configpipxtremewidget.cpp
    twostep.cpp
    legacy-calibration.cpp
    gyro-calibration.cpp
    alignment-calibration.cpp
    defaultattitudewidget.cpp
    defaulthwsettingswidget.cpp
    inputchannelform.cpp
    configcamerastabilizationwidget.cpp
    configrevowidget.cpp
    configtxpidwidget.cpp
    cfg_vehicletypes/vehicleconfig.cpp
    cfg_vehicletypes/configccpmwidget.cpp
    cfg_vehicletypes/configmultirotorwidget.cpp
    cfg_vehicletypes/configfixedwingwidget.cpp
    cfg_vehicletypes/configgroundvehiclewidget.cpp
    cfg_vehicletypes/configcustomwidget.cpp
    outputchannelform.cpp
    mixercurve.cpp
    dblspindelegate.cpp
    configrevohwwidget.cpp
)

qt5_wrap_ui(Config_SRCS
    airframe.ui
    airframe_ccpm.ui
    airframe_fixedwing.ui
    airframe_ground.ui
    airframe_multirotor.ui
    airframe_custom.ui
    cc_hw_settings.ui
    stabilization.ui
    input.ui
    output.ui
    ccattitude.ui
    defaultattitude.ui
    defaulthwsettings.ui
    inputchannelform.ui
    camerastabilization.ui
    outputchannelform.ui
    revosensors.ui
    txpid.ui
    pipxtreme.ui
    mixercurve.ui
    configrevohwwidget.ui
)

qt5_add_resources(Config_SRCS
    configgadget.qrc
) 

add_library(Config ${Config_SRCS})

target_link_libraries(Config Core UAVObjectWidgetUtils)

qt5_use_modules(Config Widgets Xml)

add_definitions(-DCONFIG_LIBRARY)

configure_file(Config.pluginspec ${PROJECT_BINARY_DIR}/ground/openpilotgcs/src/plugins/ COPYONLY)
