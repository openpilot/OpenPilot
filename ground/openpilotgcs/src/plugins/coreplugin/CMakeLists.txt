include_directories(actionmanager dialogs uavgadgetmanager)

add_definitions(-DCORE_LIBRARY)

set(Core_SRCS
    mainwindow.cpp
    tabpositionindicator.cpp
    fancyactionbar.cpp
    fancytabwidget.cpp
    generalsettings.cpp
    uniqueidmanager.cpp
    messagemanager.cpp
    messageoutputwindow.cpp
    versiondialog.cpp
    iuavgadget.cpp
    uavgadgetmanager/uavgadgetmanager.cpp
    uavgadgetmanager/uavgadgetview.cpp
    uavgadgetmanager/splitterorview.cpp
    actionmanager/actionmanager.cpp
    actionmanager/command.cpp
    actionmanager/actioncontainer.cpp
    actionmanager/commandsfile.cpp
    dialogs/settingsdialog.cpp
    dialogs/shortcutsettings.cpp
    basemode.cpp
    baseview.cpp
    coreplugin.cpp
    variablemanager.cpp
    threadmanager.cpp
    modemanager.cpp
    coreimpl.cpp
    plugindialog.cpp
    manhattanstyle.cpp
    minisplitter.cpp
    styleanimator.cpp
    rightpane.cpp
    sidebar.cpp
    mimedatabase.cpp
    icore.cpp
    dialogs/ioptionspage.cpp
    dialogs/iwizard.cpp
    settingsdatabase.cpp
    eventfilteringmainwindow.cpp
    connectionmanager.cpp
    iconnection.cpp
    iuavgadgetconfiguration.cpp
    uavgadgetinstancemanager.cpp
    uavgadgetoptionspagedecorator.cpp
    uavgadgetdecorator.cpp
    workspacesettings.cpp
    uavconfiginfo.cpp
    authorsdialog.cpp
    telemetrymonitorwidget.cpp
)

set(Core_ITFS
    icontext.h
    imode.h
    iview.h
    ioutputpane.h
    iuavgadgetfactory.h
    iconfigurableplugin.h
    icorelistener.h
)

qt5_wrap_ui(Core_SRCS
    dialogs/settingsdialog.ui
    dialogs/shortcutsettings.ui
    generalsettings.ui
    uavgadgetoptionspage.ui
    workspacesettings.ui
)

qt5_add_resources(Core_SRCS
    core.qrc
    fancyactionbar.qrc
) 

add_library(Core ${Core_SRCS} ${Core_ITFS})

target_link_libraries(Core utils opmapcontrol Aggregation UAVObjects version_info)

qt5_use_modules(Core Widgets Xml)

configure_file(Core.pluginspec ${PROJECT_BINARY_DIR}/ground/openpilotgcs/src/plugins/ COPYONLY)
