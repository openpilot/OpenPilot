set(LineardialGadget_SRCS
    lineardialplugin.cpp
    lineardialgadget.cpp
    lineardialgadgetfactory.cpp
    lineardialgadgetwidget.cpp
    lineardialgadgetconfiguration.cpp
    lineardialgadgetoptionspage.cpp
)

qt5_wrap_ui(LineardialGadget_SRCS
    lineardialgadgetoptionspage.ui
)

qt5_add_resources(LineardialGadget_SRCS
    lineardial.qrc
) 

add_library(LineardialGadget ${LineardialGadget_SRCS})

if(WIN32)
    set_target_properties(LineardialGadget PROPERTIES PREFIX "")
endif()

target_link_libraries(LineardialGadget Core)

qt5_use_modules(LineardialGadget Widgets)

configure_file(LineardialGadget.pluginspec ${GCS_PLUGINS_DIR} COPYONLY)

