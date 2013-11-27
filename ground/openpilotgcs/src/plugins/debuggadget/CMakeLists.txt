set(DebugGadget_SRCS
    debugplugin.cpp
    debugengine.cpp
    debuggadget.cpp
    debuggadgetfactory.cpp
    debuggadgetwidget.cpp
)

qt5_wrap_ui(DebugGadget_SRCS debug.ui)

add_library(DebugGadget ${DebugGadget_SRCS})

if(WIN32)
    set_target_properties(DebugGadget PROPERTIES PREFIX "")
endif()

target_link_libraries(DebugGadget Core)

qt5_use_modules(DebugGadget Widgets)

configure_file(DebugGadget.pluginspec ${GCS_PLUGINS_DIR} COPYONLY)

