include_directories(
    ../shared/qtsingleapplication
    ../libs
)
  
add_definitions(-DGCS_LIBRARY_BASENAME="${GCS_LIBRARY_BASENAME}")

set(gcs_SRCS
    main.cpp
    gcssplashscreen.cpp
    openpilotgcs.rc
)

qt5_add_resources(gcs_SRCS
    appresources.qrc
) 

add_executable(openpilotgcs WIN32 ${gcs_SRCS})

target_link_libraries(openpilotgcs ExtensionSystem sharedtools version_info utils)

qt5_use_modules(openpilotgcs Widgets Xml)

add_custom_command(TARGET openpilotgcs POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/openpilotgcs/share ${CMAKE_BINARY_DIR}/share
)
