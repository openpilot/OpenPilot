set(SystemHealthGadget_SRCS
    systemhealthplugin.cpp
    systemhealthgadget.cpp
    systemhealthgadgetfactory.cpp
    systemhealthgadgetwidget.cpp
    systemhealthgadgetconfiguration.cpp
    systemhealthgadgetoptionspage.cpp
)

qt5_wrap_ui(SystemHealthGadget_SRCS
    systemhealthgadgetoptionspage.ui
)

qt5_add_resources(SystemHealthGadget_SRCS
    systemhealth.qrc
) 

add_library(SystemHealthGadget ${SystemHealthGadget_SRCS})

target_link_libraries(SystemHealthGadget Core UAVTalk)

qt5_use_modules(SystemHealthGadget Widgets)

configure_file(SystemHealthGadget.pluginspec ${PROJECT_BINARY_DIR}/ground/openpilotgcs/src/plugins/ COPYONLY)

