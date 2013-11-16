set(CMAKE_PREFIX_PATH
    $(CMAKE_PREFIX_PATH)
    ${PROJECT_BINARY_DIR}/tools/qt-5.1.1/5.1.1/gcc_64/lib/cmake/
)

find_package(Qt5Core)
find_package(Qt5Widgets)
find_package(Qt5Declarative)
