set(UAVObjects_SRCS
    uavobject.cpp
    uavmetaobject.cpp
    uavobjectmanager.cpp
    uavdataobject.cpp
    uavobjectfield.cpp
    uavobjectsplugin.cpp
)

file(GLOB uavobjsynthetics_SRCS ${PROJECT_BINARY_DIR}/uavobject-synthetics/gcs/*.cpp)

add_library(UAVObjects ${UAVObjects_SRCS} ${uavobjsynthetics_SRCS})

# target_link_libraries(UAVObjects Core)

add_dependencies(UAVObjects uavobject-synthetics)

qt5_use_modules(UAVObjects Core)

configure_file(UAVObjects.pluginspec ${PROJECT_BINARY_DIR}/ground/openpilotgcs/src/plugins/ COPYONLY)

