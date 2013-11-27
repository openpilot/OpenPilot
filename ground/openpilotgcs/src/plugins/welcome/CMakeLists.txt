set(Welcome_SRCS
    welcomeplugin.cpp
    welcomemode.cpp
)

qt5_add_resources(Welcome_SRCS welcome.qrc)

add_library(Welcome ${Welcome_SRCS})

if(WIN32)
    set_target_properties(Welcome PROPERTIES PREFIX "")
endif()

qt5_use_modules(Welcome Widgets)

target_link_libraries(Welcome Core)

add_definitions(-DWELCOME_LIBRARY)

configure_file(Welcome.pluginspec ${GCS_PLUGINS_DIR}/Welcome.pluginspec COPYONLY)
