set(GCSControl_SRCS
    gcscontrolgadget.cpp
    gcscontrolgadgetconfiguration.cpp
    gcscontrolgadgetoptionspage.cpp
    gcscontrolgadgetwidget.cpp
    gcscontrolgadgetfactory.cpp
    gcscontrolplugin.cpp
    joystickcontrol.cpp
)

qt5_wrap_ui(GCSControl_SRCS
    gcscontrol.ui
    gcscontrolgadgetoptionspage.ui
)

qt5_add_resources(GCSControl_SRCS
    gcscontrol.qrc
)

add_library(GCSControl ${GCSControl_SRCS})

target_link_libraries(GCSControl Core UAVObjects sdlgamepad)

qt5_use_modules(GCSControl Widgets Network)

configure_file(GCSControl.pluginspec ${PROJECT_BINARY_DIR}/ground/openpilotgcs/src/plugins/ COPYONLY)

