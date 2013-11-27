set(LoggingGadget_SRCS
    loggingplugin.cpp
    logfile.cpp
    logginggadgetwidget.cpp
    logginggadget.cpp
    logginggadgetfactory.cpp
)

qt5_wrap_ui(LoggingGadget_SRCS
    logging.ui
)

add_library(LoggingGadget ${LoggingGadget_SRCS})

if(WIN32)
    set_target_properties(LoggingGadget PROPERTIES PREFIX "")
endif()

target_link_libraries(LoggingGadget Core ScopeGadget)

qt5_use_modules(LoggingGadget Widgets)

configure_file(LoggingGadget.pluginspec ${GCS_PLUGINS_DIR} COPYONLY)

