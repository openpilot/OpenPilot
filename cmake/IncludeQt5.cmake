set(CMAKE_PREFIX_PATH
    $(CMAKE_PREFIX_PATH)
    ${PROJECT_BINARY_DIR}/tools/qt-5.1.1/5.1.1/gcc_64/lib/cmake/
    ${PROJECT_BINARY_DIR}/tools/qt-5.1.1/5.1.1/mingw48_32/lib/cmake/
    "C:/Qt/Qt5.1.1/5.1.1/msvc2010_opengl/lib/cmake/"
    "C:/Program Files (x86)/Microsoft SDKs/Windows/v7.0A/Lib"
)

find_package(OpenGL REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Declarative REQUIRED)
