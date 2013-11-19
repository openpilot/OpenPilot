set(Welcome_SRCS
    welcomeplugin.cpp
    welcomemode.cpp
)

qt5_add_resources(Welcome_SRCS welcome.qrc)

add_library(Welcome ${Welcome_SRCS})

qt5_use_modules(Welcome Widgets)

target_link_libraries(Welcome Core)

add_definitions(-DWELCOME_LIBRARY)

configure_file(Welcome.pluginspec ${PROJECT_BINARY_DIR}/ground/openpilotgcs/src/plugins/ COPYONLY)

