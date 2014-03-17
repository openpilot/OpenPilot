macx {
    # Ensures that SDL framework and header files are found when compiled with Qt5.1.1
    INCLUDEPATH += /Library/Frameworks/SDL.framework/Headers
    SDL = -F/Library/Frameworks
    # Add SDL to CFLAGS fixes build problems on mac
    QMAKE_CFLAGS += $$SDL
    QMAKE_CXXFLAGS += $$SDL
    # Let the linker know where to find the frameworks
    LIBS += $$SDL
}

win32 {
    INCLUDEPATH += $(SDL_DIR)/include
}

LIBS *= -l$$qtLibraryName(sdlgamepad)
