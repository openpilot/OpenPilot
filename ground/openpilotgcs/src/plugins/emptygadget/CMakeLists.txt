set(EmptyGadget_SRCS
    emptyplugin.cpp
    emptygadget.cpp
    emptygadgetfactory.cpp
    emptygadgetwidget.cpp
)

add_library(EmptyGadget ${EmptyGadget_SRCS})

if(WIN32)
    set_target_properties(EmptyGadget PROPERTIES PREFIX "")
endif()

target_link_libraries(EmptyGadget Core)

qt5_use_modules(EmptyGadget Widgets)

configure_file(EmptyGadget.pluginspec ${GCS_PLUGINS_DIR} COPYONLY)

