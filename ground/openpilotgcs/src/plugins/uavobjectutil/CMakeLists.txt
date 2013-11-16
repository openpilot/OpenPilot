set(UAVObjectUtil_SRCS
    uavobjectutilmanager.cpp
    uavobjectutilplugin.cpp
)

add_library(UAVObjectUtil ${UAVObjectUtil_SRCS})

# qt5_use_modules(UAVObjectUtil Widgets)

target_link_libraries(UAVObjectUtil Core UAVObjects)

configure_file(UAVObjectUtil.pluginspec ${PROJECT_BINARY_DIR}/ground/openpilotgcs/src/plugins/ COPYONLY)


