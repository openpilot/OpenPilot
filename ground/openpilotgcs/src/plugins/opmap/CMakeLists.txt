set(OPMapGadget_SRCS
    opmapplugin.cpp
    opmapgadgetwidget.cpp
    opmapgadgetoptionspage.cpp
    opmapgadgetfactory.cpp
    opmapgadgetconfiguration.cpp
    opmapgadget.cpp
    opmap_edit_waypoint_dialog.cpp
    opmap_zoom_slider_widget.cpp
    opmap_statusbar_widget.cpp
    flightdatamodel.cpp
    modelmapproxy.cpp
    widgetdelegates.cpp
    pathplanner.cpp
    modeluavoproxy.cpp
    homeeditor.cpp
)

qt5_wrap_ui(OPMapGadget_SRCS
    opmapgadgetoptionspage.ui
    opmap_widget.ui
    opmap_edit_waypoint_dialog.ui
    opmap_zoom_slider_widget.ui
    opmap_statusbar_widget.ui
    opmap_overlay_widget.ui
    pathplanner.ui
    homeeditor.ui
)

qt5_add_resources(OPMapGadget_SRCS
    opmap.qrc
) 

add_library(OPMapGadget ${OPMapGadget_SRCS})

if(WIN32)
    set_target_properties(OPMapGadget PROPERTIES PREFIX "")
endif()

target_link_libraries(OPMapGadget Core UAVObjectUtil UAVTalk)

qt5_use_modules(OPMapGadget Widgets Xml)

configure_file(OPMapGadget.pluginspec ${GCS_PLUGINS_DIR} COPYONLY)

