set(IPconnection_SRCS
    ipconnectionplugin.cpp
    ipconnectionconfiguration.cpp
    ipconnectionoptionspage.cpp
    ipconnection_internal.h
)

qt5_wrap_ui(IPconnection_SRCS
    ipconnectionoptionspage.ui
)

add_library(IPconnection ${IPconnection_SRCS})

if(WIN32)
    set_target_properties(IPconnection PROPERTIES PREFIX "")
endif()

target_link_libraries(IPconnection Core)

qt5_use_modules(IPconnection Widgets)

add_definitions(-DIPconnection_LIBRARY)

configure_file(IPconnection.pluginspec ${GCS_PLUGINS_DIR} COPYONLY)

