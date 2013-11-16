set(GpsDisplayGadget_SRCS
    gpsdisplayplugin.cpp
    gpsconstellationwidget.cpp
    gpsparser.cpp
    telemetryparser.cpp
    gpssnrwidget.cpp
    buffer.cpp
    nmeaparser.cpp
    gpsdisplaygadget.cpp
    gpsdisplaygadgetfactory.cpp
    gpsdisplaywidget.cpp
    gpsdisplaygadgetconfiguration.cpp
    gpsdisplaygadgetoptionspage.cpp
)

qt5_wrap_ui(GpsDisplayGadget_SRCS
    gpsdisplaygadgetoptionspage.ui
    gpsdisplaywidget.ui
)

qt5_add_resources(GpsDisplayGadget_SRCS
    widgetresources.qrc
) 

add_library(GpsDisplayGadget ${GpsDisplayGadget_SRCS})

target_link_libraries(GpsDisplayGadget Core)

qt5_use_modules(GpsDisplayGadget Widgets)

configure_file(GpsDisplayGadget.pluginspec ${PROJECT_BINARY_DIR}/ground/openpilotgcs/src/plugins/ COPYONLY)

