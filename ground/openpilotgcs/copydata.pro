include(openpilotgcs.pri)

TEMPLATE = subdirs

# Copy Qt runtime libraries into the build directory (to run or package)
equals(copyqt, 1) {

GCS_LIBRARY_PATH

    linux-* {

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

        # create QtQuick2 plugin directories
        QT_QUICK2_DIRS = QtQuick \
                         QtQuick.2 \
                         QtQuick/Layouts \
                         QtQuick/LocalStorage \
                         QtQuick/Particles.2 \
                         QtQuick/PrivateWidgets \
                         QtQuick/Window.2 \
                         QtQuick/XmlListModel
        for(dir, QT_QUICK2_DIRS) {
            data_copy.commands += -@$(MKDIR) $$targetPath(\"$$GCS_QT_QML_PATH/$$dir\") $$addNewline()
        }

        # Copy QtQuick2 complete directories
        # These directories have a lot of files
        # Easier to copy everything
        QTQ_WHOLE_DIRS = QtQuick/Controls \
                         QtQuick/Dialogs
        for(dir, QTQ_WHOLE_DIRS) {
            data_copy.commands += $(COPY_DIR) $$targetPath(\"$$[QT_INSTALL_QML]/$$dir\") $$targetPath(\"$$GCS_QT_QML_PATH/$$dir\") $$addNewline()
        }

        # Remaining QtQuick plugin libs
        QT_QUICK2_DLLS = QtQuick.2/libqtquick2plugin.so \
                         QtQuick.2/plugins.qmltypes \
                         QtQuick.2/qmldir \
        QtQuick/Layouts/libqquicklayoutsplugin.so \
        QtQuick/Layouts/plugins.qmltypes \
        QtQuick/Layouts/qmldir \
        QtQuick/LocalStorage/libqmllocalstorageplugin.so \
        QtQuick/LocalStorage/plugins.qmltypes \
        QtQuick/LocalStorage/qmldir \
        QtQuick/Particles.2/libparticlesplugin.so \
        QtQuick/Particles.2/plugins.qmltypes \
        QtQuick/Particles.2/qmldir \
        QtQuick/PrivateWidgets/libwidgetsplugin.so  \
        QtQuick/PrivateWidgets/plugins.qmltypes \
        QtQuick/PrivateWidgets/qmldir \
        QtQuick/Window.2/libwindowplugin.so \
        QtQuick/Window.2/plugins.qmltypes \
        QtQuick/Window.2/qmldir \
        QtQuick/XmlListModel/libqmlxmllistmodelplugin.so \
        QtQuick/XmlListModel/plugins.qmltypes \
        QtQuick/XmlListModel/qmldir

        for(lib, QT_QUICK2_DLLS) {
            data_copy.commands += $(COPY_FILE) $$targetPath(\"$$[QT_INSTALL_QML]/$$lib\") $$targetPath(\"$$GCS_QT_QML_PATH/$$lib\") $$addNewline()
        }

        data_copy.target = FORCE
        QMAKE_EXTRA_TARGETS += data_copy
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

        # create QtQuick2 plugin directories
        QT_QUICK2_DIRS = qtquick \
                         qtquick.2 \
                         qtquick/layouts \
                         qtquick/localstorage \
                         qtquick/particles.2 \
                         qtquick/privatewidgets \
                         qtquick/window.2 \
                         qtquick/xmllistmodel
        for(dir, QT_QUICK2_DIRS) {
            data_copy.commands += -@$(MKDIR) $$targetPath(\"$$GCS_APP_PATH/$$dir\") $$addNewline()
        }

        # Copy QtQuick2 complete directories
        # These directories have a lot of files
        # Easier to copy everything
        QTQ_WHOLE_DIRS = qtquick/controls \
                         qtquick/dialogs
        for(dir, QTQ_WHOLE_DIRS) {
            data_copy.commands += $(COPY_DIR) $$targetPath(\"$$[QT_INSTALL_QML]/$$dir\") $$targetPath(\"$$GCS_APP_PATH/$$dir\") $$addNewline()
        }

        # Remaining QtQuick plugin DLLs
        QT_QUICK2_DLLS = QtQuick.2/qtquick2plugin$${DS}.dll \
                         QtQuick.2/plugins.qmltypes \
                         QtQuick.2/qmldir \
        qtquick/layouts/qquicklayoutsplugin$${DS}.dll \
        qtquick/layouts/plugins.qmltypes \
        qtquick/layouts/qmldir \
        qtquick/localstorage/qmllocalstorageplugin$${DS}.dll \
        qtquick/localstorage/plugins.qmltypes \
        qtquick/localstorage/qmldir \
        qtquick/particles.2/particlesplugin$${DS}.dll \
        qtquick/particles.2/plugins.qmltypes \
        qtquick/particles.2/qmldir \
        qtquick/privatewidgets/widgetsplugin$${DS}.dll  \
        qtquick/privatewidgets/plugins.qmltypes \
        qtquick/privatewidgets/qmldir \
        qtquick/window.2/windowplugin$${DS}.dll \
        qtquick/window.2/plugins.qmltypes \
        qtquick/window.2/qmldir \
        qtquick/XmlListModel/qmlxmllistmodelplugin$${DS}.dll \
        qtquick/XmlListModel/plugins.qmltypes \
        qtquick/XmlListModel/qmldir

        for(dll, QT_QUICK2_DLLS) {
            data_copy.commands += $(COPY_FILE) $$targetPath(\"$$[QT_INSTALL_QML]/$$dll\") $$targetPath(\"$$GCS_APP_PATH/$$dll\") $$addNewline()
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

        data_copy.target = FORCE
        QMAKE_EXTRA_TARGETS += data_copy
    }


    macx{
        #NOTE: debug dylib can be copied as they will be cleaned out with packaging scripts
        #standard plugins directory (will copy just dylib, plugins.qmltypes and qmldir
        QT_QUICK2_PLUGINS = QtQuick.2 QtQuick/Layouts QtQuick/LocalStorage QtQuick/Particles.2 QtQuick/PrivateWidgets QtQuick/Window.2 QtQuick/XmlListModel
        #those directories will be fully copied to dest
        QT_QUICK2_FULL_DIRS = QtQuick/Controls QtQuick/Dialogs

        #create QtQuick dir (that will host all subdirs)
        data_copy.commands += -@$(MKDIR) $$targetPath(\"$$GCS_QT_QML_PATH/QtQuick\") $$addNewline()

        for(dir, QT_QUICK2_FULL_DIRS) {
            #data_copy.commands += -@$(MKDIR) $$targetPath(\"$$GCS_QT_QML_PATH/$$dir\") $$addNewline()
            data_copy.commands += $(COPY_DIR) $$targetPath(\"$$[QT_INSTALL_QML]/$$dir\") $$targetPath(\"$$GCS_QT_QML_PATH/$$dir\") $$addNewline()
        }

        for(lib, QT_QUICK2_PLUGINS) {
            data_copy.commands += $(MKDIR) $$targetPath(\"$$GCS_QT_QML_PATH/$$lib\") $$addNewline()
            data_copy.commands += $(COPY_FILE) $$targetPath(\"$$[QT_INSTALL_QML]/$$lib/\"*.dylib) $$targetPath(\"$$GCS_QT_QML_PATH/$$lib/\") $$addNewline()
            data_copy.commands += $(COPY_FILE) $$targetPath(\"$$[QT_INSTALL_QML]/$$lib/plugins.qmltypes\") $$targetPath(\"$$GCS_QT_QML_PATH/$$lib/plugins.qmltypes\") $$addNewline()
            data_copy.commands += $(COPY_FILE) $$targetPath(\"$$[QT_INSTALL_QML]/$$lib/qmldir\") $$targetPath(\"$$GCS_QT_QML_PATH/$$lib/qmldir\") $$addNewline()
        }

        data_copy.target = FORCE
        QMAKE_EXTRA_TARGETS += data_copy
    }

    win32:CONFIG(debug, debug|release) {

        # copy Qt DLLs and phonon4
        QT_DLLS = phonond4.dll \
                  QtCored4.dll \
                  QtGuid4.dll \
                  QtNetworkd4.dll \
                  QtOpenGLd4.dll \
                  QtSqld4.dll \
                  QtSvgd4.dll \
                  QtTestd4.dll \
                  QtXmld4.dll \
                  QtDeclaratived4.dll \
                  QtXmlPatternsd4.dll \
                  QtScriptd4.dll
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
        QT_ICONENGINE_DLLS = qsvgicond4.dll
        data_copy.commands += -@$(MKDIR) $$targetPath(\"$$GCS_APP_PATH/iconengines\") $$addNewline()
        for(dll, QT_ICONENGINE_DLLS) {
            data_copy.commands += $(COPY_FILE) $$targetPath(\"$$[QT_INSTALL_PLUGINS]/iconengines/$$dll\") $$targetPath(\"$$GCS_APP_PATH/iconengines/$$dll\") $$addNewline()
        }

        # copy imageformats
        QT_IMAGEFORMAT_DLLS = qgifd4.dll qicod4.dll qjpegd4.dll qmngd4.dll qsvgd4.dll qtiffd4.dll
        data_copy.commands += -@$(MKDIR) $$targetPath(\"$$GCS_APP_PATH/imageformats\") $$addNewline()
        for(dll, QT_IMAGEFORMAT_DLLS) {
            data_copy.commands += $(COPY_FILE) $$targetPath(\"$$[QT_INSTALL_PLUGINS]/imageformats/$$dll\") $$targetPath(\"$$GCS_APP_PATH/imageformats/$$dll\") $$addNewline()
        }

        # copy phonon_backend
        QT_PHONON_BACKEND_DLLS = phonon_ds9d4.dll
        data_copy.commands += -@$(MKDIR) $$targetPath(\"$$GCS_APP_PATH/phonon_backend\") $$addNewline()
        for(dll, QT_PHONON_BACKEND_DLLS) {
            data_copy.commands += $(COPY_FILE) $$targetPath(\"$$[QT_INSTALL_PLUGINS]/phonon_backend/$$dll\") $$targetPath(\"$$GCS_APP_PATH/phonon_backend/$$dll\") $$addNewline()
        }

        # copy sqldrivers
        QT_SQLDRIVERS_DLLS = qsqlited4.dll
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
