equals(copydata, 1) {
    win32 {
        # copy SDL DLL
        SDL_DLLS = \
            SDL.dll
        for(dll, SDL_DLLS) {
            addCopyFileTarget($${SDL_DIR}/bin/$${dll},$${GCS_APP_PATH}/$${dll})
        }
    }
}
