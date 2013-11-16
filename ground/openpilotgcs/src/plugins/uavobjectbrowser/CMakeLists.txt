set(UAVObjectBrowser_SRCS
    browserplugin.cpp
    uavobjectbrowserconfiguration.cpp
    uavobjectbrowser.cpp
    uavobjectbrowserfactory.cpp
    uavobjectbrowserwidget.cpp
    uavobjectbrowseroptionspage.cpp
    uavobjecttreemodel.cpp
    treeitem.cpp
    browseritemdelegate.cpp
    fieldtreeitem.cpp
)

set(UAVObjectBrowser_UIS
    uavobjectbrowser.ui
    uavobjectbrowseroptionspage.ui
    viewoptions.ui
)

qt5_wrap_ui(UAVObjectBrowser_SRCS ${UAVObjectBrowser_UIS})

qt5_add_resources(UAVObjectBrowser_SRCS uavobjectbrowser.qrc)

add_library(UAVObjectBrowser ${UAVObjectBrowser_SRCS})

add_dependencies(UAVObjectBrowser Core uavobject-synthetics)

qt5_use_modules(UAVObjectBrowser Widgets)

target_link_libraries(UAVObjectBrowser Core UAVObjects)

configure_file(UAVObjectBrowser.pluginspec ${PROJECT_BINARY_DIR}/ground/openpilotgcs/src/plugins/ COPYONLY)

