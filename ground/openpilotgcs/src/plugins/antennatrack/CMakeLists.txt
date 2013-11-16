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

target_link_libraries(AntennaTrack Core UAVObjects)

qt5_use_modules(AntennaTrack Widgets SerialPort)

configure_file(AntennaTrack.pluginspec ${PROJECT_BINARY_DIR}/ground/openpilotgcs/src/plugins/ COPYONLY)

