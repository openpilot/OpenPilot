set(EmptyGadget_SRCS
    emptyplugin.cpp
    emptygadget.cpp
    emptygadgetfactory.cpp
    emptygadgetwidget.cpp
)

add_library(EmptyGadget ${EmptyGadget_SRCS})

target_link_libraries(EmptyGadget Core)

qt5_use_modules(EmptyGadget Widgets)

configure_file(EmptyGadget.pluginspec ${PROJECT_BINARY_DIR}/ground/openpilotgcs/src/plugins/ COPYONLY)

