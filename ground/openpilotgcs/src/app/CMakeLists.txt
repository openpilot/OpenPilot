include_directories(
    ${CMAKE_SOURCE_DIR}/ground/openpilotgcs/src/shared/qtsingleapplication
    ${CMAKE_SOURCE_DIR}/ground/openpilotgcs/src/libs
)

set(GCS_LIBRARY_BASENAME lib)
  
add_definitions(-DGCS_LIBRARY_BASENAME="${GCS_LIBRARY_BASENAME}")

set(gcs_SRCS
    main.cpp
    gcssplashscreen.cpp
)

qt5_add_resources(gcs_SRCS
    appresources.qrc
) 

add_executable(openpilotgcs ${gcs_SRCS})

target_link_libraries(openpilotgcs ExtensionSystem sharedtools version_info gcsutils)

qt5_use_modules(openpilotgcs Widgets Xml)

add_custom_command(TARGET openpilotgcs PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/ground/openpilotgcs/share ${CMAKE_BINARY_DIR}/ground/openpilotgcs/src/share
)
