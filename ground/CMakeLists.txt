set(CMAKE_PREFIX_PATH
    $(CMAKE_PREFIX_PATH)
    ${PROJECT_BINARY_DIR}/tools/qt-5.1.1/5.1.1/gcc_64/lib/cmake/
    ${PROJECT_BINARY_DIR}/tools/qt-5.1.1/5.1.1/mingw48_32/lib/cmake/
    ${PROJECT_BINARY_DIR}/tools/qt-5.1.1/5.1.1/msvc2010_opengl/lib/cmake/
    "C:/Program Files (x86)/Microsoft SDKs/Windows/v7.0A/Lib"
)

# Find Qt5
find_package(OpenGL REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Declarative REQUIRED)

# Find SDL
find_package(Sdl REQUIRED)

if(WIN32)
    # Find HID
    find_package(HID REQUIRED)
endif()

set(GCS_BINARY_DIR ${PROJECT_BINARY_DIR}/bin)
set(GCS_LIBRARY_BASENAME lib)
set(GCS_PLUGINS_DIR ${PROJECT_BINARY_DIR}/${GCS_LIBRARY_BASENAME}/openpilotgcs/plugins)

set(EXECUTABLE_OUTPUT_PATH ${GCS_BINARY_DIR})
foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${GCS_BINARY_DIR}) # TODO needed?
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${GCS_BINARY_DIR}) # TODO needed?
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${GCS_BINARY_DIR}) # TODO needed?   
    set(EXECUTABLE_OUTPUT_PATH_${OUTPUTCONFIG} ${GCS_BINARY_DIR})
endforeach(OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES)

if(WIN32)
    # Copy DLLs
    get_filename_component(QT5_BINARY_DIR ${QT_MOC_EXECUTABLE} PATH)
    
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(D "d")
    endif()
    
    set(QT5_DLLS
        Qt5Core${D}.dll
        Qt5Gui${D}.dll
        Qt5Widgets${D}.dll
        Qt5Network${D}.dll
        Qt5OpenGL${D}.dll
        Qt5Sql${D}.dll
        Qt5Svg${D}.dll
        Qt5Test${D}.dll
        Qt5Xml${D}.dll
        Qt5Declarative${D}.dll
        Qt5XmlPatterns${D}.dll
        Qt5Script${D}.dll
        Qt5Concurrent${D}.dll
        Qt5PrintSupport${D}.dll
        Qt5OpenGL${D}.dll
        Qt5SerialPort${D}.dll
        Qt5Multimedia${D}.dll
        Qt5MultimediaWidgets${D}.dll
        icuin51.dll
        icudt51.dll
        icuuc51.dll
    )
    
    if(MINGW)
        set(QT5_DLLS
            ${QT5_DLLS}
            libgcc_s_dw2-1.dll
            libstdc++-6.dll
            libwinpthread-1.dll
        )
    endif()
    
    set(QT5_DIRS
        iconengines
        imageformats
        platforms
        mediaservice
        sqldrivers
    )
    
    foreach(FILENAME ${QT5_DLLS})
        set(SRC ${QT5_BINARY_DIR}/${FILENAME})
        set(DST ${GCS_BINARY_DIR}/${FILENAME})
        add_custom_command(
            OUTPUT ${DST}
            COMMAND ${CMAKE_COMMAND} -E copy ${SRC} ${DST}
            COMMENT "Copying ${SRC} "
        )
        set(QT5_DEPENDENCIES ${QT5_DEPENDENCIES} "${DST}")
    endforeach()
    
    foreach(DIR ${QT5_DIRS})
        set(SRC ${QT5_BINARY_DIR}/../plugins/${DIR})
        set(DST ${GCS_BINARY_DIR}/${DIR})
        add_custom_command(
            OUTPUT ${DST}
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${SRC} ${DST}
            COMMENT "Copying directory ${SRC}"
        )
        set(QT5_DEPENDENCIES ${QT5_DEPENDENCIES} "${DST}")
    endforeach()
    
    get_filename_component(SDL_LIBNAME ${SDL_LIBRARY_PATH} NAME)
    set(SDL_DEPENDENCIES ${GCS_BINARY_DIR}/${SDL_LIBNAME})
    add_custom_command(
        OUTPUT ${SDL_DEPENDENCIES}
        COMMAND ${CMAKE_COMMAND} -E copy ${SDL_LIBRARY_PATH} ${SDL_DEPENDENCIES}
        COMMENT "Copying ${SDL_LIBRARY_PATH}"
    )
    
    add_custom_target (copydata ALL DEPENDS ${QT5_DEPENDENCIES} ${SDL_DEPENDENCIES})
endif()

add_subdirectory(uavobjgenerator)
add_subdirectory(uavobject-synthetics)
add_subdirectory(openpilotgcs)
