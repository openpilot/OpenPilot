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

if(WIN32)
    set_target_properties(GCSControl PROPERTIES PREFIX "")
endif()

target_link_libraries(GCSControl Core sdlgamepad)

qt5_use_modules(GCSControl Widgets Network)

configure_file(GCSControl.pluginspec ${GCS_PLUGINS_DIR} COPYONLY)

