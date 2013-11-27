set(DialGadget_SRCS
    dialplugin.cpp
    dialgadget.cpp
    dialgadgetfactory.cpp
    dialgadgetwidget.cpp
    dialgadgetconfiguration.cpp
    dialgadgetoptionspage.cpp
)

qt5_wrap_ui(DialGadget_SRCS
    dialgadgetoptionspage.ui
)

qt5_add_resources(DialGadget_SRCS
    dial.qrc
) 

add_library(DialGadget ${DialGadget_SRCS})

if(WIN32)
    set_target_properties(DialGadget PROPERTIES PREFIX "")
endif()

target_link_libraries(DialGadget Core)

qt5_use_modules(DialGadget Widgets)

configure_file(DialGadget.pluginspec ${GCS_PLUGINS_DIR} COPYONLY)

