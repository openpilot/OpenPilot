set(uavsettingsimportexport_SRCS
    uavsettingsimportexport.cpp
    importsummary.cpp
    uavsettingsimportexportfactory.cpp
)

qt5_wrap_ui(uavsettingsimportexport_SRCS importsummarydialog.ui)

add_library(UAVSettingsImportExport ${uavsettingsimportexport_SRCS} ${uavobjsynthetics_SRCS})

target_link_libraries(UAVSettingsImportExport Core UAVObjectUtil)

qt5_use_modules(UAVSettingsImportExport Core Widgets Xml)

add_definitions(-DUAVSETTINGSIMPORTEXPORT_LIBRARY)

configure_file(uavsettingsimportexport.pluginspec ${PROJECT_BINARY_DIR}/ground/openpilotgcs/src/plugins/ COPYONLY)

