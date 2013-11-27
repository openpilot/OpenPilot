set(version_info_SRC ${PROJECT_BINARY_DIR}/ground/openpilotgcs/src/libs/version_info/version_info.cpp)
set(version_info_TPL ${PROJECT_SOURCE_DIR}/ground/openpilotgcs/src/libs/version_info/version_info.cpp.template)
set(version_info_SCR ${PROJECT_SOURCE_DIR}/make/scripts/version-info.py)

add_custom_command(
    OUTPUT ${version_info_SRC}
    COMMAND ${version_info_SCR}
        --path="${PROJECT_SOURCE_DIR}/ground/openpilotgcs"
        --template="${version_info_TPL}"
        --uavodir="${PROJECT_BINARY_DIR}/uavobject-synthetics"
        --outfile="${version_info_SRC}"
    DEPENDS ${version_info_SCR} ${version_info_TPL}
)

add_library(version_info STATIC ${version_info_SRC})

add_dependencies(version_info uavobject-synthetics)

qt5_use_modules(version_info Core)
