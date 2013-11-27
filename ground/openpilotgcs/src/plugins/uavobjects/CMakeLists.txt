include(${PROJECT_SOURCE_DIR}/ground/uavobject-synthetics/uavobject.cmake)

add_definitions(-DUAVOBJECTS_LIBRARY)

set(UAVObjects_SRCS
    uavobject.cpp
    uavmetaobject.cpp
    uavobjectmanager.cpp
    uavdataobject.cpp
    uavobjectfield.cpp
    uavobjectsplugin.cpp
)

qt5_wrap_cpp(uavobjgcs_MOCS ${uavobjgcs_HDRS})

add_library(UAVObjects ${UAVObjects_SRCS} ${uavobjgcs_SRCS} ${uavobjgcs_MOCS})

if(WIN32)
    set_target_properties(UAVObjects PROPERTIES PREFIX "")
endif()

target_link_libraries(UAVObjects ExtensionSystem)

qt5_use_modules(UAVObjects Core)

configure_file(UAVObjects.pluginspec ${GCS_PLUGINS_DIR} COPYONLY)
