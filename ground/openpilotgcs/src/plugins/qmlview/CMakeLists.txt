set(QMLView_SRCS
    qmlviewplugin.cpp
    qmlviewgadget.cpp
    qmlviewgadgetfactory.cpp
    qmlviewgadgetwidget.cpp
    qmlviewgadgetconfiguration.cpp
    qmlviewgadgetoptionspage.cpp
)

qt5_wrap_ui(QMLView_SRCS
    qmlviewgadgetoptionspage.ui
)

add_library(QMLView ${QMLView_SRCS})

if(WIN32)
    set_target_properties(QMLView PROPERTIES PREFIX "")
endif()

target_link_libraries(QMLView Core UAVTalk qwt)

qt5_use_modules(QMLView Widgets Svg)

configure_file(QMLView.pluginspec ${GCS_PLUGINS_DIR} COPYONLY)

