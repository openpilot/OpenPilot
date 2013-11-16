include_directories(..)

add_definitions(-DGCS_TEST_DIR="${PROJECT_SOURCE_DIR}")

set(extensionsystem_SRCS
    pluginerrorview.cpp
    plugindetailsview.cpp
    iplugin.cpp
    pluginmanager.cpp
    pluginspec.cpp
    pluginview.cpp
    optionsparser.cpp
)

qt5_wrap_ui(extensionsystem_SRCS
    pluginerrorview.ui
    plugindetailsview.ui
    pluginview.ui
)

qt5_add_resources(extensionsystem_SRCS
    pluginview.qrc
) 

add_library(ExtensionSystem ${extensionsystem_SRCS})

set_target_properties(ExtensionSystem PROPERTIES 
    VERSION ${GCSLIBS_VERSION_STRING} 
    SOVERSION ${GCSLIBS_VERSION_MAJOR}
)                                       
                                          
add_dependencies(ExtensionSystem uavobject-synthetics)

qt5_use_modules(ExtensionSystem Widgets)
