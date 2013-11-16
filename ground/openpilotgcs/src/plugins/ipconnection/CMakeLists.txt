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

target_link_libraries(IPconnection Core)

qt5_use_modules(IPconnection Widgets)

configure_file(IPconnection.pluginspec ${PROJECT_BINARY_DIR}/ground/openpilotgcs/src/plugins/ COPYONLY)

