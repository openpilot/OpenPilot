equals(copydata, 1) {

    win32 {
        # copy SDL DLL
        SDL_DLLS = \
            SDL.dll
        for(dll, SDL_DLLS) {
            data_copy.commands += $(COPY_FILE) $$targetPath(\"$$(SDL_DIR)/bin/$$dll\") $$targetPath(\"$$GCS_APP_PATH/$$dll\") $$addNewline()
        }

        # add make target
        POST_TARGETDEPS += copydata

        data_copy.target = copydata
        QMAKE_EXTRA_TARGETS += data_copy
    }

}
