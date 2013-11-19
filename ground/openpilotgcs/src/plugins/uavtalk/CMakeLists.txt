set(UAVTalk_SRCS
    uavtalk.cpp
    uavtalkplugin.cpp
    telemetrymonitor.cpp
    telemetrymanager.cpp
    telemetry.cpp
)

add_library(UAVTalk ${UAVTalk_SRCS})

add_dependencies(UAVTalk uavobject-synthetics)

qt5_use_modules(UAVTalk Widgets)

target_link_libraries(UAVTalk Core UAVObjectBrowser)

add_definitions(-DUAVTALK_LIBRARY)

configure_file(UAVTalk.pluginspec ${PROJECT_BINARY_DIR}/ground/openpilotgcs/src/plugins/ COPYONLY)

