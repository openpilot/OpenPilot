set(DoNothing_SRCS
    donothingplugin.cpp
)

add_library(DoNothing ${DoNothing_SRCS})

if(WIN32)
    set_target_properties(DoNothing PROPERTIES PREFIX "")
endif()

target_link_libraries(DoNothing Core)

qt5_use_modules(DoNothing Widgets)

configure_file(DoNothing.pluginspec ${GCS_PLUGINS_DIR} COPYONLY)

