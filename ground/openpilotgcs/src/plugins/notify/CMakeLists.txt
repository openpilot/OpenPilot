set(NotifyPlugin_SRCS
    notifyplugin.cpp  
    notifypluginoptionspage.cpp
    notifyitemdelegate.cpp
    notifytablemodel.cpp
    notificationitem.cpp
    notifylogging.cpp
)

qt5_wrap_ui(NotifyPlugin_SRCS
    notifypluginoptionspage.ui
)

qt5_add_resources(NotifyPlugin_SRCS
    res.qrc
) 

add_library(NotifyPlugin ${NotifyPlugin_SRCS})

target_link_libraries(NotifyPlugin Core UAVTalk)

qt5_use_modules(NotifyPlugin Widgets Svg MultimediaWidgets)

configure_file(NotifyPlugin.pluginspec ${PROJECT_BINARY_DIR}/ground/openpilotgcs/src/plugins/ COPYONLY)

