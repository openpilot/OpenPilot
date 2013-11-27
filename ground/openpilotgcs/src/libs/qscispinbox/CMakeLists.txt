set(ScienceSpinBox_SRCS
    QScienceSpinBox.cpp
)

add_library(ScienceSpinBox STATIC ${ScienceSpinBox_SRCS})

qt5_use_modules(ScienceSpinBox Core Widgets)

set_target_properties(ScienceSpinBox PROPERTIES 
    VERSION ${GCSLIBS_VERSION_STRING} 
    SOVERSION ${GCSLIBS_VERSION_MAJOR}
)     
