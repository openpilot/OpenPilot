set(AntennaTrack_SRCS
    antennatrackplugin.cpp
    gpsparser.cpp
    telemetryparser.cpp
    antennatrackgadget.cpp
    antennatrackgadgetfactory.cpp
    antennatrackwidget.cpp
    antennatrackgadgetconfiguration.cpp
    antennatrackgadgetoptionspage.cpp
)

qt5_wrap_ui(AntennaTrack_SRCS
    antennatrackgadgetoptionspage.ui
    antennatrackwidget.ui
)

add_library(AntennaTrack ${AntennaTrack_SRCS})

if(WIN32)
    set_target_properties(AntennaTrack PROPERTIES PREFIX "")
endif()

target_link_libraries(AntennaTrack Core UAVObjects)

qt5_use_modules(AntennaTrack Widgets SerialPort)

configure_file(AntennaTrack.pluginspec ${GCS_PLUGINS_DIR} COPYONLY)

