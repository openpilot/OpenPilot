set(Serial_SRCS
    serialplugin.cpp
    serialpluginconfiguration.cpp
    serialpluginoptionspage.cpp
)

set(Serial_UIS
    serialpluginoptions.ui
)

qt5_wrap_ui(Serial_SRCS ${Serial_UIS})

add_library(Serial ${Serial_SRCS})

target_link_libraries(Serial Core)

qt5_use_modules(Serial Widgets SerialPort)

configure_file(Serial.pluginspec ${PROJECT_BINARY_DIR}/ground/openpilotgcs/src/plugins/ COPYONLY)

