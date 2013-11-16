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

target_link_libraries(ImportExportGadget Core)

qt5_use_modules(ImportExportGadget Widgets Xml)

configure_file(ImportExportGadget.pluginspec ${PROJECT_BINARY_DIR}/ground/openpilotgcs/src/plugins/ COPYONLY)

