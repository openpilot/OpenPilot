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

target_link_libraries(DialGadget Core)

qt5_use_modules(DialGadget Widgets)

configure_file(DialGadget.pluginspec ${PROJECT_BINARY_DIR}/ground/openpilotgcs/src/plugins/ COPYONLY)

