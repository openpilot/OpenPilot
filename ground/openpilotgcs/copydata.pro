include(openpilotgcs.pri)

TEMPLATE = subdirs

# Copy Qt runtime libraries into the build directory (to run or package)
equals(copyqt, 1) {

    # Copy QtQuick2 complete directories
    # Some of these directories have a lot of files
    # Easier to copy everything
    QT_QUICK2_DIRS = QtQuick/Controls \
                     QtQuick/Dialogs \
                     QtQuick/Layouts \
                     QtQuick/LocalStorage \
                     QtQuick/Particles.2 \
                     QtQuick/PrivateWidgets \
                     QtQuick/Window.2 \
                     QtQuick/XmlListModel \
                     QtQuick.2

    # create QtQuick directory
    data_copy.commands += -@$(MKDIR) $$targetPath(\"$$GCS_QT_QML_PATH/QtQuick\") $$addNewline()

    for(dir, QT_QUICK2_DIRS) {
        data_copy.commands += @rm -rf $$targetPath(\"$$GCS_QT_QML_PATH/$$dir\") $$addNewline()
        data_copy.commands += $(COPY_DIR) $$targetPath(\"$$[QT_INSTALL_QML]/$$dir\") $$targetPath(\"$$GCS_QT_QML_PATH/$$dir\") $$addNewline()
    }

    data_copy.target = FORCE
    QMAKE_EXTRA_TARGETS += data_copy

    linux {

        QT_LIBS = libQt5Core.so.5 \
                  libQt5Gui.so.5 \
                  libQt5Widgets.so.5 \
                  libQt5Network.so.5 \
                  libQt5OpenGL.so.5 \
                  libQt5Sql.so.5 \
                  libQt5Svg.so.5 \
                  libQt5Test.so.5 \
                  libQt5Xml.so.5 \
                  libQt5Declarative.so.5 \
                  libQt5XmlPatterns.so.5 \
                  libQt5Script.so.5 \
                  libQt5Concurrent.so.5 \
                  libQt5PrintSupport.so.5 \
                  libQt5SerialPort.so.5 \
                  libQt5Multimedia.so.5 \
                  libQt5MultimediaWidgets.so.5 \
                  libQt5Quick.so.5 \
                  libQt5Qml.so.5 \
                  libQt5DBus.so.5 \
                  libQt5QuickParticles.so.5 \
                  libqgsttools_p.so.1 \
                  libicui18n.so.53 \
                  libicuuc.so.53 \
                  libicudata.so.53

        data_copy.commands += -@$(MKDIR) $$targetPath(\"$$GCS_QT_LIBRARY_PATH\") $$addNewline()
        for(lib, QT_LIBS) {
            data_copy.commands += $(COPY_FILE) $$targetPath(\"$$[QT_INSTALL_LIBS]/$$lib\") $$targetPath(\"$$GCS_QT_LIBRARY_PATH/$$lib\") $$addNewline()
        }

        # create Qt plugin directories
        QT_PLUGIN_DIRS = iconengines \
                         imageformats \
                         platforms \
                         mediaservice \
                         sqldrivers
        for(dir, QT_PLUGIN_DIRS) {
            data_copy.commands += -@$(MKDIR) $$targetPath(\"$$GCS_QT_PLUGINS_PATH/$$dir\") $$addNewline()
        }
        QT_PLUGIN_LIBS = iconengines/libqsvgicon.so \
                         imageformats/libqgif.so \
                         imageformats/libqico.so \
                         imageformats/libqjpeg.so \
                         imageformats/libqmng.so \
                         imageformats/libqsvg.so \
                         imageformats/libqtiff.so \
                         mediaservice/libgstaudiodecoder.so \
                         mediaservice/libgstmediaplayer.so \
                         platforms/libqxcb.so \
                         sqldrivers/libqsqlite.so
        for(lib, QT_PLUGIN_LIBS) {
            data_copy.commands += $(COPY_FILE) $$targetPath(\"$$[QT_INSTALL_PLUGINS]/$$lib\") $$targetPath(\"$$GCS_QT_PLUGINS_PATH/$$lib\") $$addNewline()
        }
    }

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
                  icuin53.dll \
                  icudt53.dll \
                  icuuc53.dll
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
                         sqldrivers \
			 opengl32_32
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

        # copy OpenSSL DLLs
        OPENSSL_DLLS = \
            ssleay32.dll \
            libeay32.dll
        for(dll, OPENSSL_DLLS) {
            data_copy.commands += $(COPY_FILE) $$targetPath(\"$${OPENSSL_DIR}/$$dll\") $$targetPath(\"$$GCS_APP_PATH/$$dll\") $$addNewline()
        }

        # copy OpenGL DLL
        OPENGL_DLLS = \
            opengl32_32/opengl32.dll
        for(dll, OPENGL_DLLS) {
            data_copy.commands += $(COPY_FILE) $$targetPath(\"$${MESAWIN_DIR}/$$dll\") $$targetPath(\"$$GCS_APP_PATH/$$dll\") $$addNewline()
        }
    }
}
