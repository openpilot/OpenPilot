set(UAVObjectWidgetUtils_SRCS
    uavobjectwidgetutilsplugin.cpp
    configtaskwidget.cpp
    mixercurvewidget.cpp
    mixercurvepoint.cpp
    mixercurveline.cpp
    smartsavebutton.cpp
    popupwidget.cpp
)

qt5_add_resources(UAVObjectWidgetUtils_SRCS uavobjectwidgetutils.qrc)

add_library(UAVObjectWidgetUtils ${UAVObjectWidgetUtils_SRCS} ${uavobjsynthetics_SRCS})

target_link_libraries(UAVObjectWidgetUtils Core UAVSettingsImportExport UAVTalk)

qt5_use_modules(UAVObjectWidgetUtils Core Widgets)

add_definitions(-DUAVOBJECTWIDGETUTILS_LIBRARY)

configure_file(UAVObjectWidgetUtils.pluginspec ${PROJECT_BINARY_DIR}/ground/openpilotgcs/src/plugins/ COPYONLY)

