equals(copydata, 1) {
    win32 {
        # copy SDL DLL
        SDL_DLLS = \
            SDL.dll
        for(dll, SDL_DLLS) {
            addCopyFileTarget($${dll},$${SDL_DIR}/bin,$${GCS_APP_PATH})
        }
    }
}
