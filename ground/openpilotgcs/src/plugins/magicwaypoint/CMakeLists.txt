set(MagicWaypoint_SRCS
    magicwaypointgadget.cpp
    magicwaypointgadgetwidget.cpp
    magicwaypointgadgetfactory.cpp
    magicwaypointplugin.cpp
    positionfield.cpp
)

qt5_wrap_ui(MagicWaypoint_SRCS
    magicwaypoint.ui
)

qt5_add_resources(MagicWaypoint_SRCS
    magicwaypoint.qrc
) 

add_library(MagicWaypoint ${MagicWaypoint_SRCS})

if(WIN32)
    set_target_properties(MagicWaypoint PROPERTIES PREFIX "")
endif()

target_link_libraries(MagicWaypoint Core)

qt5_use_modules(MagicWaypoint Widgets)

configure_file(MagicWaypoint.pluginspec ${GCS_PLUGINS_DIR} COPYONLY)

