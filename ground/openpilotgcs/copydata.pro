include(openpilotgcs.pri)

TEMPLATE = subdirs

# Copy Qt runtime libraries into the build directory (to run or package)
equals(copydata, 1) {

    # Windows release only, no debug target DLLs ending with 'd'
    # It is assumed that SDL.dll can be found in the same directory as mingw32-make.exe
    win32:CONFIG(release, debug|release) {

        # copy Qt DLLs and phonon4
        QT_DLLS = phonon4.dll \
                  QtCore4.dll \
                  QtGui4.dll \
                  QtNetwork4.dll \
                  QtOpenGL4.dll \
                  QtSql4.dll \
                  QtSvg4.dll \
                  QtTest4.dll \
                  QtXml4.dll \
                  QtDeclarative4.dll \
                  QtXmlPatterns4.dll \
                  QtScript4.dll
        for(dll, QT_DLLS) {
            data_copy.commands += $(COPY_FILE) $$targetPath(\"$$[QT_INSTALL_BINS]/$$dll\") $$targetPath(\"$$GCS_APP_PATH/$$dll\") $$addNewline()
        }

        # copy MinGW DLLs
        MINGW_DLLS = libgcc_s_dw2-1.dll \
                     mingwm10.dll
        for(dll, MINGW_DLLS) {
            data_copy.commands += $(COPY_FILE) $$targetPath(\"$$(QTMINGW)/$$dll\") $$targetPath(\"$$GCS_APP_PATH/$$dll\") $$addNewline()
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

        # copy SDL - Simple DirectMedia Layer (www.libsdl.org)
        # Check the wiki for SDL installation, it should be copied first
        # (make sure that the Qt installation path below is correct)
        #
        # - For qt-sdk-win-opensource-2010.05.exe:
        #   xcopy /s /e <SDL>\bin\SDL.dll   C:\Qt\2010.05\mingw\bin\SDL.dll
        #   xcopy /s /e <SDL>\include\SDL\* C:\Qt\2010.05\mingw\include\SDL
        #   xcopy /s /e <SDL>\lib\*         C:\Qt\2010.05\mingw\lib
        #
        # - For Qt_SDK_Win_offline_v1_1_1_en.exe:
        #   xcopy /s /e <SDL>\bin\SDL.dll   C:\QtSDK\Desktop\Qt\4.7.3\mingw\bin\SDL.dll
        #   xcopy /s /e <SDL>\include\SDL\* C:\QtSDK\Desktop\Qt\4.7.3\mingw\include\SDL
        #   xcopy /s /e <SDL>\lib\*         C:\QtSDK\Desktop\Qt\4.7.3\mingw\lib
        SDL_DLL = SDL.dll
        data_copy.commands += $(COPY_FILE) $$targetPath(\"$$(QTMINGW)/$$SDL_DLL\") $$targetPath(\"$$GCS_APP_PATH/$$SDL_DLL\") $$addNewline()

        data_copy.target = FORCE
        QMAKE_EXTRA_TARGETS += data_copy
    }
}
