include(openpilotgcs.pri)

TEMPLATE = subdirs

# Copy Qt runtime libraries into the build directory (to run or package)
equals(copydata, 1) {

    # Windows release only, no debug target DLLs ending with 'd'
    # It is assumed that SDL.dll can be found in the same directory as mingw32-make.exe
    win32 {

        # set debug suffix if needed
        CONFIG(debug, debug|release):DS = "d"

        # copy Qt DLLs
        QT_DLLS = Qt5Core$${DS}.dll \
                  Qt5Gui$${DS}.dll \
                  Qt5Widgets$${DS}.dll \
                  Qt5Network$${DS}.dll \
                  Qt5OpenGL$${DS}.dll \
                  Qt5Sql$${DS}.dll \
                  Qt5Svg$${DS}.dll \
                  Qt5Test$${DS}.dll \
                  Qt5Xml$${DS}.dll \
                  Qt5Declarative$${DS}.dll \
                  Qt5XmlPatterns$${DS}.dll \
                  Qt5Script$${DS}.dll \
                  Qt5Concurrent$${DS}.dll \
                  Qt5PrintSupport$${DS}.dll \
                  Qt5OpenGL$${DS}.dll \
                  Qt5SerialPort$${DS}.dll \
                  Qt5Multimedia$${DS}.dll \
                  Qt5MultimediaWidgets$${DS}.dll \
                  icuin51.dll \
                  icudt51.dll \
                  icuuc51.dll
        # it is more robust to take the following DLLs from Qt rather than from MinGW
        QT_DLLS += libgcc_s_dw2-1.dll \
                   libstdc++-6.dll \
                   libwinpthread-1.dll
        for(dll, QT_DLLS) {
            data_copy.commands += $(COPY_FILE) $$targetPath(\"$$[QT_INSTALL_BINS]/$$dll\") $$targetPath(\"$$GCS_APP_PATH/$$dll\") $$addNewline()
        }

        # create Qt plugin directories
        QT_PLUGIN_DIRS = iconengines \
                         imageformats \
                         platforms \
                         mediaservice \
                         sqldrivers
        for(dir, QT_PLUGIN_DIRS) {
            data_copy.commands += -@$(MKDIR) $$targetPath(\"$$GCS_APP_PATH/$$dir\") $$addNewline()
        }

        # copy Qt plugin DLLs
        QT_PLUGIN_DLLS = iconengines/qsvgicon$${DS}.dll \
                         imageformats/qgif$${DS}.dll \
                         imageformats/qico$${DS}.dll \
                         imageformats/qjpeg$${DS}.dll \
                         imageformats/qmng$${DS}.dll \
                         imageformats/qsvg$${DS}.dll \
                         imageformats/qtiff$${DS}.dll \
                         platforms/qwindows$${DS}.dll \
                         mediaservice/dsengine$${DS}.dll \
                         sqldrivers/qsqlite$${DS}.dll
        for(dll, QT_PLUGIN_DLLS) {
            data_copy.commands += $(COPY_FILE) $$targetPath(\"$$[QT_INSTALL_PLUGINS]/$$dll\") $$targetPath(\"$$GCS_APP_PATH/$$dll\") $$addNewline()
        }

<<<<<<< HEAD
        # copy MinGW DLLs
        MINGW_DLLS = SDL.dll
        for(dll, MINGW_DLLS) {
            data_copy.commands += $(COPY_FILE) $$targetPath(\"$$(QTMINGW)/$$dll\") $$targetPath(\"$$GCS_APP_PATH/$$dll\") $$addNewline()
        }

=======
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
        3PARTYDLL_DLLS = SDL.dll ssleay32.dll libeay32.dll
        for(dll, 3PARTYDLL_DLLS) {
            data_copy.commands += $(COPY_FILE) $$targetPath(\"$$(QTMINGW)/$$dll\") $$targetPath(\"$$GCS_APP_PATH/$$dll\") $$addNewline()
        }
        
>>>>>>> master
        data_copy.target = FORCE
        QMAKE_EXTRA_TARGETS += data_copy
    }

}
