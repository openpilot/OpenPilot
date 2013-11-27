project(OpenPilot)

cmake_minimum_required(VERSION 2.8.11)

set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    ${CMAKE_CURRENT_SOURCE_DIR}/make
)

add_subdirectory(ground)
