include_directories(
    ${PROJECT_SOURCE_DIR}/ground/openpilotgcs/src/libs
    ${PROJECT_SOURCE_DIR}/ground/openpilotgcs/src/plugins
    ${PROJECT_SOURCE_DIR}/ground/openpilotgcs/src/plugins/uavobjects
    ${PROJECT_SOURCE_DIR}/ground/openpilotgcs/src/plugins/uavobjectutil
    ${PROJECT_BINARY_DIR}/uavobject-synthetics/gcs
)

if(MINGW)
    include_directories(${PROJECT_BINARY_DIR}/tools/qt-5.1.1/Tools/mingw48_32/i686-w64-mingw32/include)
endif()

set(BUILD_SHARED_LIBS ON)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/ground/openpilotgcs/src/plugins)

add_subdirectory(antennatrack)
add_subdirectory(config)
add_subdirectory(coreplugin)
add_subdirectory(debuggadget)
add_subdirectory(dial)
# add_subdirectory(donothing) # we keep it as an example
add_subdirectory(emptygadget)
add_subdirectory(gcscontrol)
add_subdirectory(gpsdisplay)
add_subdirectory(hitl)
add_subdirectory(importexport)
add_subdirectory(ipconnection)
add_subdirectory(lineardial)
add_subdirectory(logging)
add_subdirectory(magicwaypoint)
add_subdirectory(modelview)
add_subdirectory(notify)
add_subdirectory(ophid)
add_subdirectory(opmap)
# add_subdirectory(osgearthview) # later
add_subdirectory(pathactioneditor)
add_subdirectory(pfdqml)
add_subdirectory(qmlview)
add_subdirectory(scope)
add_subdirectory(serialconnection)
add_subdirectory(setupwizard)
add_subdirectory(systemhealth)
add_subdirectory(uavobjectbrowser)
add_subdirectory(uavobjects)
add_subdirectory(uavobjectutil)
add_subdirectory(uavobjectwidgetutils)
add_subdirectory(uavsettingsimportexport)
add_subdirectory(uavtalk)
add_subdirectory(uploader)
add_subdirectory(welcome)