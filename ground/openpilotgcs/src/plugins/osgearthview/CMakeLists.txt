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

target_link_libraries(OsgEarthviewGadget Core)

qt5_use_modules(OsgEarthviewGadget Widgets OpenGL)

configure_file(OsgEarthviewGadget.pluginspec ${PROJECT_BINARY_DIR}/ground/openpilotgcs/src/plugins/ COPYONLY)

