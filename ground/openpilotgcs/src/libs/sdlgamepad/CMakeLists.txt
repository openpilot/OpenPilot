message(STATUS "Looking for SDL ")
find_package(Sdl)
if(SDL_FOUND)
    include_directories(${SDL_INCLUDE_DIR})
    add_definitions(-DJOYSTICKS)
else()
    message("SDL not found! Joysticks support will be disabled")
endif()

set(sdlgamepad_SRCS
    sdlgamepad.cpp
)

add_library(sdlgamepad ${sdlgamepad_SRCS})

qt5_use_modules(sdlgamepad Core)

target_link_libraries(sdlgamepad ${SDL_LIBRARY})
