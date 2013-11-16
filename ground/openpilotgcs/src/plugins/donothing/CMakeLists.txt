set(DoNothing_SRCS
    donothingplugin.cpp
)

add_library(DoNothing ${DoNothing_SRCS})

target_link_libraries(DoNothing Core)

qt5_use_modules(DoNothing Widgets)

configure_file(DoNothing.pluginspec ${PROJECT_BINARY_DIR}/ground/openpilotgcs/src/plugins/ COPYONLY)

