set(ImportExportGadget_SRCS
    importexportplugin.cpp
    importexportgadgetwidget.cpp
    importexportdialog.cpp
)

qt5_wrap_ui(ImportExportGadget_SRCS
    importexportdialog.ui
    importexportgadgetwidget.ui
)

add_library(ImportExportGadget ${ImportExportGadget_SRCS})

if(WIN32)
    set_target_properties(ImportExportGadget PROPERTIES PREFIX "")
endif()

target_link_libraries(ImportExportGadget Core)

qt5_use_modules(ImportExportGadget Widgets Xml)

add_definitions(-DIMPORTEXPORT_LIBRARY)

configure_file(ImportExportGadget.pluginspec ${GCS_PLUGINS_DIR} COPYONLY)

