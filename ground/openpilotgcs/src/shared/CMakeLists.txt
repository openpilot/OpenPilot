include_directories(qtlockedfile)

#add_definitions(-DGCS_TEST_DIR="${PROJECT_SOURCE_DIR}")

set(sharedtools_SRCS
    qtlockedfile/qtlockedfile_unix.cpp
    qtlockedfile/qtlockedfile.cpp
    qtsingleapplication/qtsingleapplication.cpp
    qtsingleapplication/qtlocalpeer.cpp
)

add_library(sharedtools ${sharedtools_SRCS})

#add_dependencies(extensionsystem uavobject-synthetics)

qt5_use_modules(sharedtools Widgets Network)
