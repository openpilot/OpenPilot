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

target_link_libraries(LoggingGadget Core ScopeGadget)

qt5_use_modules(LoggingGadget Widgets)

configure_file(LoggingGadget.pluginspec ${PROJECT_BINARY_DIR}/ground/openpilotgcs/src/plugins/ COPYONLY)

