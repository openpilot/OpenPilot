set(UAVObjectUtil_SRCS
    uavobjectutilmanager.cpp
    uavobjectutilplugin.cpp
)

add_library(UAVObjectUtil ${UAVObjectUtil_SRCS})

if(WIN32)
    set_target_properties(UAVObjectUtil PROPERTIES PREFIX "")
endif()

target_link_libraries(UAVObjectUtil Core UAVObjects)

add_definitions(-DUAVOBJECTUTIL_LIBRARY)

configure_file(UAVObjectUtil.pluginspec ${GCS_PLUGINS_DIR} COPYONLY)


