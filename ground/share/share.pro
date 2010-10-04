include(../openpilotgcs.pri)

TEMPLATE = subdirs
SUBDIRS = openpilotgcs/translations

DATACOLLECTIONS = dials models pfd sounds diagrams mapicons

equals(copydata, 1) {
    for(dir, DATACOLLECTIONS) {
        exists($$GCS_SOURCE_TREE/share/openpilotgcs/$$dir) {
            macx:data_copy.commands += $(COPY_DIR) $$targetPath(\"$$GCS_SOURCE_TREE/share/openpilotgcs/$$dir\") $$targetPath(\"$$GCS_DATA_PATH/\") $$addNewline()
            !macx:data_copy.commands += $(COPY_DIR) $$targetPath(\"$$GCS_SOURCE_TREE/share/openpilotgcs/$$dir\") $$targetPath(\"$$GCS_DATA_PATH/$$dir\") $$addNewline()
        }
    }

    # copying required for installer QT DLLs
    win32 {
        !debug {
            QT_DLLs = libgcc_s_dw2-1.dll mingwm10.dll phonon4.dll QtCore4.dll QtGui4.dll QtNetwork4.dll QtOpenGL4.dll QtSql4.dll QtSvg4.dll QtTest4.dll QtXml4.dll
            for(dll, QT_DLLs) {
                data_copy.commands += $(COPY_FILE) $$targetPath(\"$$[QT_INSTALL_BINS]/$$dll\") $$targetPath(\"$$GCS_APP_PATH/$$dll\") $$addNewline()
            }

            # copy imageformats
            QT_ImageFormats_DLLs = qgif4.dll qico4.dll qjpeg4.dll qmng4.dll qsvg4.dll qtiff4.dll
            for(dll, QT_ImageFormats_DLLs) {
                data_copy.commands += $(COPY_FILE) $$targetPath(\"$$[QT_INSTALL_PLUGINS]/imageformats/$$dll\") $$targetPath(\"$$GCS_APP_PATH/$$dll\") $$addNewline()
            }

            # copy iconengine
            data_copy.commands += $(COPY_FILE) $$targetPath(\"$$[QT_INSTALL_PLUGINS]/iconengines/qsvgicon4.dll\") $$targetPath(\"$$GCS_APP_PATH/qsvgicon4.dll\") $$addNewline()

            # copy SDL (if available) - Simple DirectMedia Layer (www.libsdl.org)
            SDL_DLL = $$targetPath(\"$$[QT_INSTALL_BINS]/../../mingw/bin/SDL.dll\")
            exists($$SDL_DLL){
                data_copy.commands += $(COPY_FILE) $$SDL_DLL $$targetPath(\"$$GCS_APP_PATH/SDL.dll\") $$addNewline()
            }

            # copy sql driver
            data_copy.commands += @$(CHK_DIR_EXISTS) $$targetPath(\"$$GCS_APP_PATH/sqldrivers\") $(MKDIR) $$targetPath(\"$$GCS_APP_PATH/sqldrivers\") $$addNewline()
            data_copy.commands += $(COPY_FILE) $$targetPath(\"$$[QT_INSTALL_PLUGINS]/sqldrivers/qsqlite4.dll\") $$targetPath(\"$$GCS_APP_PATH/sqldrivers/qsqlite4.dll\") $$addNewline()
        }
    }

    data_copy.target = FORCE
    QMAKE_EXTRA_TARGETS += data_copy                   
}
