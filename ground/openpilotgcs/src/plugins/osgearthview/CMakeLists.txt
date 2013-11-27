set(OsgEarthviewGadget_SRCS
    osgearthviewplugin.cpp
    osgviewerwidget.cpp
    osgearthviewgadget.cpp
    osgearthviewwidget.cpp
    osgearthviewgadgetfactory.cpp
    osgearthviewgadgetconfiguration.cpp
    osgearthviewgadgetoptionspage.cpp
)

qt5_wrap_ui(OsgEarthviewGadget_SRCS
    osgearthviewgadgetoptionspage.ui
    osgearthview.ui
)

qt5_add_resources(OsgEarthviewGadget_SRCS
    osgearthview.qrc
) 

add_library(OsgEarthviewGadget ${OsgEarthviewGadget_SRCS})

if(WIN32)
    set_target_properties(OsgEarthviewGadget PROPERTIES PREFIX "")
endif()

target_link_libraries(OsgEarthviewGadget Core)

qt5_use_modules(OsgEarthviewGadget Widgets OpenGL)

configure_file(OsgEarthviewGadget.pluginspec ${GCS_PLUGINS_DIR} COPYONLY)

