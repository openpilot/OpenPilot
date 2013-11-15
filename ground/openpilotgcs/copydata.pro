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
                  Qt5Quick$${DS}.dll \
                  Qt5Qml$${DS}.dll \
                  Qt5V8$${DS}.dll \
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

        # copy MinGW DLLs
        MINGW_DLLS = SDL.dll
        for(dll, MINGW_DLLS) {
            data_copy.commands += $(COPY_FILE) $$targetPath(\"$$(QTMINGW)/$$dll\") $$targetPath(\"$$GCS_APP_PATH/$$dll\") $$addNewline()
        }

        data_copy.target = FORCE
        QMAKE_EXTRA_TARGETS += data_copy
    }

}
