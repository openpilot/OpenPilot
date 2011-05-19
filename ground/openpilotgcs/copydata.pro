include(openpilotgcs.pri)

TEMPLATE = subdirs

# Copy Qt runtime libraries into the build directory (to run or package)
equals(copydata, 1) {

    # Windows release only, no debug target DLLs ending with 'd'
    win32:CONFIG(release, debug|release) {

        # copy Qt DLLs and phonon4
        QT_DLLS = libgcc_s_dw2-1.dll \
                  mingwm10.dll \
                  phonon4.dll \
                  QtCore4.dll \
                  QtGui4.dll \
                  QtNetwork4.dll \
                  QtOpenGL4.dll \
                  QtSql4.dll \
                  QtSvg4.dll \
                  QtTest4.dll \
                  QtXml4.dll
        for(dll, QT_DLLS) {
            data_copy.commands += $(COPY_FILE) $$targetPath(\"$$[QT_INSTALL_BINS]/$$dll\") $$targetPath(\"$$GCS_APP_PATH/$$dll\") $$addNewline()
        }

        # copy iconengines
        QT_ICONENGINE_DLLS = qsvgicon4.dll
        data_copy.commands += -@$(MKDIR) $$targetPath(\"$$GCS_APP_PATH/iconengines\") $$addNewline()
        for(dll, QT_ICONENGINE_DLLS) {
            data_copy.commands += $(COPY_FILE) $$targetPath(\"$$[QT_INSTALL_PLUGINS]/iconengines/$$dll\") $$targetPath(\"$$GCS_APP_PATH/iconengines/$$dll\") $$addNewline()
        }

        # copy imageformats
        QT_IMAGEFORMAT_DLLS = qgif4.dll qico4.dll qjpeg4.dll qmng4.dll qsvg4.dll qtiff4.dll
        data_copy.commands += -@$(MKDIR) $$targetPath(\"$$GCS_APP_PATH/imageformats\") $$addNewline()
        for(dll, QT_IMAGEFORMAT_DLLS) {
            data_copy.commands += $(COPY_FILE) $$targetPath(\"$$[QT_INSTALL_PLUGINS]/imageformats/$$dll\") $$targetPath(\"$$GCS_APP_PATH/imageformats/$$dll\") $$addNewline()
        }

        # copy phonon_backend
        QT_PHONON_BACKEND_DLLS = phonon_ds94.dll
        data_copy.commands += -@$(MKDIR) $$targetPath(\"$$GCS_APP_PATH/phonon_backend\") $$addNewline()
        for(dll, QT_PHONON_BACKEND_DLLS) {
            data_copy.commands += $(COPY_FILE) $$targetPath(\"$$[QT_INSTALL_PLUGINS]/phonon_backend/$$dll\") $$targetPath(\"$$GCS_APP_PATH/phonon_backend/$$dll\") $$addNewline()
        }

        # copy sqldrivers
        QT_SQLDRIVERS_DLLS = qsqlite4.dll
        data_copy.commands += -@$(MKDIR) $$targetPath(\"$$GCS_APP_PATH/sqldrivers\") $$addNewline()
        for(dll, QT_SQLDRIVERS_DLLS) {
            data_copy.commands += $(COPY_FILE) $$targetPath(\"$$[QT_INSTALL_PLUGINS]/sqldrivers/$$dll\") $$targetPath(\"$$GCS_APP_PATH/sqldrivers/$$dll\") $$addNewline()
        }

        # copy SDL (if available) - Simple DirectMedia Layer (www.libsdl.org)
        SDL_DLL = SDL.dll
        exists($$targetPath(\"$$[QT_INSTALL_BINS]/../../mingw/bin/$$SDL_DLL\")) {
            data_copy.commands += $(COPY_FILE) $$targetPath(\"$$[QT_INSTALL_BINS]/../../mingw/bin/$$SDL_DLL\") $$targetPath(\"$$GCS_APP_PATH/$$SDL_DLL\") $$addNewline()
        }

        data_copy.target = FORCE
        QMAKE_EXTRA_TARGETS += data_copy
    }
}
