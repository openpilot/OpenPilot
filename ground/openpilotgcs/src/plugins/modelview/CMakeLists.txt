include_directories(../../libs/glc_lib)

set(ModelViewGadget_SRCS
    modelviewplugin.cpp
    modelviewgadgetconfiguration.cpp
    modelviewgadget.cpp
    modelviewgadgetfactory.cpp
    modelviewgadgetwidget.cpp
    modelviewgadgetoptionspage.cpp
)

qt5_wrap_ui(ModelViewGadget_SRCS
    modelviewoptionspage.ui
)

qt5_add_resources(ModelViewGadget_SRCS
    modelview.qrc
) 

add_library(ModelViewGadget ${ModelViewGadget_SRCS})

if(WIN32)
    set_target_properties(ModelViewGadget PROPERTIES PREFIX "")
endif()

target_link_libraries(ModelViewGadget Core glc)

qt5_use_modules(ModelViewGadget Widgets Svg OpenGL)

configure_file(ModelViewGadget.pluginspec ${GCS_PLUGINS_DIR} COPYONLY)

