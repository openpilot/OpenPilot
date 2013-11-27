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

if(WIN32)
    set_target_properties(Serial PROPERTIES PREFIX "")
endif()

target_link_libraries(Serial Core)

qt5_use_modules(Serial Widgets SerialPort)

configure_file(Serial.pluginspec ${GCS_PLUGINS_DIR} COPYONLY)

