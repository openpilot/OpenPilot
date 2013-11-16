include_directories(../config)

set(SetupWizard_SRCS
    setupwizardplugin.cpp
    setupwizard.cpp
    pages/opstartpage.cpp
    pages/opendpage.cpp
    pages/controllerpage.cpp
    pages/vehiclepage.cpp
    pages/notyetimplementedpage.cpp
    pages/multipage.cpp
    pages/fixedwingpage.cpp
    pages/helipage.cpp
    pages/surfacepage.cpp
    pages/abstractwizardpage.cpp
    pages/outputpage.cpp
    pages/inputpage.cpp
    pages/summarypage.cpp
    vehicleconfigurationsource.cpp
    vehicleconfigurationhelper.cpp
    connectiondiagram.cpp
    pages/outputcalibrationpage.cpp
    outputcalibrationutil.cpp
    pages/rebootpage.cpp
    pages/savepage.cpp
    pages/autoupdatepage.cpp
    pages/revocalibrationpage.cpp
    biascalibrationutil.cpp
    pages/biascalibrationpage.cpp

)

qt5_wrap_ui(SetupWizard_SRCS
    connectiondiagram.ui
    pages/opstartpage.ui
    pages/opendpage.ui
    pages/controllerpage.ui
    pages/vehiclepage.ui
    pages/notyetimplementedpage.ui
    pages/multipage.ui
    pages/fixedwingpage.ui
    pages/helipage.ui
    pages/surfacepage.ui
    pages/outputpage.ui
    pages/inputpage.ui
    pages/summarypage.ui
    pages/outputcalibrationpage.ui
    pages/rebootpage.ui
    pages/savepage.ui
    pages/autoupdatepage.ui
    pages/revocalibrationpage.ui
    pages/biascalibrationpage.ui
)

qt5_add_resources(SetupWizard_SRCS
    wizardResources.qrc
) 

add_library(SetupWizard ${SetupWizard_SRCS})

target_link_libraries(SetupWizard Core Config Uploader)

qt5_use_modules(SetupWizard Widgets Svg)

configure_file(SetupWizard.pluginspec ${PROJECT_BINARY_DIR}/ground/openpilotgcs/src/plugins/ COPYONLY)

