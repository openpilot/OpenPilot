add_custom_command(
    OUTPUT "${PROJECT_BINARY_DIR}/ground/openpilotgcs/src/libs/version_info/version_info.cpp"
    COMMAND ${PROJECT_SOURCE_DIR}/make/scripts/version-info.py
                                    --path="${PROJECT_SOURCE_DIR}/ground/openpilotgcs"
                                    --template="${PROJECT_SOURCE_DIR}/ground/openpilotgcs/src/libs/version_info/version_info.cpp.template"
                                    --uavodir="${PROJECT_BINARY_DIR}/uavobject-synthetics"
                                    --outfile="${PROJECT_BINARY_DIR}/ground/openpilotgcs/src/libs/version_info/version_info.cpp"
    DEPENDS ${PROJECT_SOURCE_DIR}/make/scripts/version-info.py ${PROJECT_SOURCE_DIR}/ground/openpilotgcs/src/libs/version_info/version_info.cpp.template
)

set(version_info_SRCS
    ${PROJECT_BINARY_DIR}/ground/openpilotgcs/src/libs/version_info/version_info.cpp
)

add_library(version_info ${version_info_SRCS})

add_dependencies(version_info uavobject-synthetics)

qt5_use_modules(version_info Core)
