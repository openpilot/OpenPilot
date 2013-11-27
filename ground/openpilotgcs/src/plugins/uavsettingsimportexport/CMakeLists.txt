set(uavsettingsimportexport_SRCS
    uavsettingsimportexport.cpp
    importsummary.cpp
    uavsettingsimportexportfactory.cpp
)

qt5_wrap_ui(uavsettingsimportexport_SRCS importsummarydialog.ui)

add_library(UAVSettingsImportExport ${uavsettingsimportexport_SRCS} ${uavobjsynthetics_SRCS})

if(WIN32)
    set_target_properties(UAVSettingsImportExport PROPERTIES PREFIX "")
endif()

target_link_libraries(UAVSettingsImportExport Core UAVObjectUtil)

qt5_use_modules(UAVSettingsImportExport Core Widgets Xml)

add_definitions(-DUAVSETTINGSIMPORTEXPORT_LIBRARY)

configure_file(uavsettingsimportexport.pluginspec ${GCS_PLUGINS_DIR} COPYONLY)

