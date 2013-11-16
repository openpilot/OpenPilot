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

target_link_libraries(QMLView Core UAVTalk qwt)

qt5_use_modules(QMLView Widgets Svg)

configure_file(QMLView.pluginspec ${PROJECT_BINARY_DIR}/ground/openpilotgcs/src/plugins/ COPYONLY)

