set(PfdQml_SRCS
    pfdqmlplugin.cpp
    pfdqmlgadget.cpp
    pfdqmlgadgetfactory.cpp
    pfdqmlgadgetwidget.cpp
    pfdqmlgadgetconfiguration.cpp
    pfdqmlgadgetoptionspage.cpp
)

qt5_wrap_ui(PfdQml_SRCS
    pfdqmlgadgetoptionspage.ui
)

add_library(PfdQml ${PfdQml_SRCS})

if(WIN32)
    set_target_properties(PfdQml PROPERTIES PREFIX "")
endif()

target_link_libraries(PfdQml Core)

qt5_use_modules(PfdQml Widgets Svg)

configure_file(PfdQml.pluginspec ${GCS_PLUGINS_DIR} COPYONLY)

