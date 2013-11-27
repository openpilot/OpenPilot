set(sdlgamepad_SRCS
    sdlgamepad.cpp
)

add_library(sdlgamepad ${sdlgamepad_SRCS})

qt5_use_modules(sdlgamepad Core)

target_link_libraries(sdlgamepad ${SDL_LIBRARY})

add_definitions(-DSDLGAMEPAD_LIBRARY)
