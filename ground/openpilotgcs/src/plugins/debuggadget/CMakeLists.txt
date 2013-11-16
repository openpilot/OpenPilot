set(DebugGadget_SRCS
    debugplugin.cpp
    debugengine.cpp
    debuggadget.cpp
    debuggadgetfactory.cpp
    debuggadgetwidget.cpp
)

qt5_wrap_ui(DebugGadget_SRCS debug.ui)

add_library(DebugGadget ${DebugGadget_SRCS})

target_link_libraries(DebugGadget Core)

qt5_use_modules(DebugGadget Widgets)

configure_file(DebugGadget.pluginspec ${PROJECT_BINARY_DIR}/ground/openpilotgcs/src/plugins/ COPYONLY)

