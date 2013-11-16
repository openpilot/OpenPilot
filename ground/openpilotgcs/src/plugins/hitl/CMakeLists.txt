set(HITL_SRCS
    hitlplugin.cpp
    hitlwidget.cpp
    hitloptionspage.cpp
    hitlfactory.cpp
    hitlconfiguration.cpp
    hitlgadget.cpp
    hitlnoisegeneration.cpp
    simulator.cpp
    aerosimrcsimulator.cpp
    fgsimulator.cpp
    il2simulator.cpp
    xplanesimulator.cpp
)

qt5_wrap_ui(HITL_SRCS
    hitloptionspage.ui
    hitlwidget.ui
)

qt5_add_resources(HITL_SRCS
    hitlresources.qrc
) 

add_library(HITL ${HITL_SRCS})

target_link_libraries(HITL Core UAVTalk)

qt5_use_modules(HITL Widgets Network)

configure_file(hitl.pluginspec ${PROJECT_BINARY_DIR}/ground/openpilotgcs/src/plugins/ COPYONLY)

