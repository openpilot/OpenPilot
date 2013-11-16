set(PathActionEditor_SRCS
    pathactioneditorgadget.cpp
    pathactioneditorgadgetwidget.cpp
    pathactioneditorgadgetfactory.cpp
    pathactioneditorplugin.cpp
    pathactioneditortreemodel.cpp
    treeitem.cpp
    fieldtreeitem.cpp
    browseritemdelegate.cpp
)

qt5_wrap_ui(PathActionEditor_SRCS
    pathactioneditor.ui
)

qt5_add_resources(PathActionEditor_SRCS
    pathactioneditor.qrc
) 

add_library(PathActionEditor ${PathActionEditor_SRCS})

target_link_libraries(PathActionEditor Core)

qt5_use_modules(PathActionEditor Widgets)

configure_file(PathActionEditor.pluginspec ${PROJECT_BINARY_DIR}/ground/openpilotgcs/src/plugins/ COPYONLY)

