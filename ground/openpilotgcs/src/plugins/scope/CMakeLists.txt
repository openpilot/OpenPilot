set(ScopeGadget_SRCS
    scopeplugin.cpp
    plotdata.cpp
    scopegadgetoptionspage.cpp
    scopegadgetconfiguration.cpp
    scopegadget.cpp
    scopegadgetfactory.cpp
    scopegadgetwidget.cpp
)

qt5_wrap_ui(ScopeGadget_SRCS
    scopegadgetoptionspage.ui
)

add_library(ScopeGadget ${ScopeGadget_SRCS})

target_link_libraries(ScopeGadget Core UAVTalk qwt)

qt5_use_modules(ScopeGadget Widgets)

add_definitions(-DSCOPE_LIBRARY)

configure_file(ScopeGadget.pluginspec ${PROJECT_BINARY_DIR}/ground/openpilotgcs/src/plugins/ COPYONLY)

