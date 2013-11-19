set(Uploader_SRCS
    uploadergadget.cpp
    uploadergadgetconfiguration.cpp
    uploadergadgetfactory.cpp
    uploadergadgetoptionspage.cpp
    uploadergadgetwidget.cpp
    uploaderplugin.cpp
    op_dfu.cpp
    delay.cpp
    devicewidget.cpp
    SSP/port.cpp
    SSP/qssp.cpp
    SSP/qsspt.cpp
    runningdevicewidget.cpp
)

set(Uploader_UIS
    uploader.ui
    devicewidget.ui
    runningdevicewidget.ui
)

qt5_wrap_ui(Uploader_SRCS ${Uploader_UIS})

qt5_add_resources(Uploader_SRCS uploader.qrc)

add_library(Uploader ${Uploader_SRCS})

qt5_use_modules(Uploader Widgets SerialPort)

target_link_libraries(Uploader Core UAVObjects UAVObjectUtil UAVTalk opHID)

add_definitions(-DUPLOADER_LIBRARY)

configure_file(Uploader.pluginspec ${PROJECT_BINARY_DIR}/ground/openpilotgcs/src/plugins/ COPYONLY)
