equals(copydata, 1) {

    win32 {
        # copy SDL DLL
        SDL_DLLS = \
            SDL.dll
        for(dll, SDL_DLLS) {
            data_copy.commands += $(COPY_FILE) $$targetPath(\"$${SDL_DIR}/bin/$$dll\") $$targetPath(\"$$GCS_APP_PATH/$$dll\") $$addNewline()
        }

        data_copy.depends = FORCE
        QMAKE_EXTRA_TARGETS += data_copy
        PRE_TARGETDEPS += data_copy
    }

}
