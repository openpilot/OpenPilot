# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

include_directories(${SDL_INCLUDE_DIR})
include_directories(${HID_INCLUDE_DIR})

add_subdirectory(src)

