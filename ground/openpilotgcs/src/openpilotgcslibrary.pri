include(../openpilotgcs.pri)

win32 {
    DLLDESTDIR = $$GCS_APP_PATH
}

DESTDIR = $$GCS_LIBRARY_PATH

TARGET = $$qtLibraryName($$TARGET)

contains(QT_CONFIG, reduce_exports):CONFIG += hGCS_symbols

macx {
    QMAKE_LFLAGS_SONAME = -Wl,-install_name,@executable_path/../Plugins/
} else
    win32 {
        target.path = /bin
        target.files = $$DESTDIR/$${TARGET}.dll
    } else {
        QMAKE_RPATHDIR = \'\$$ORIGIN\'
        QMAKE_RPATHDIR += \'\$$ORIGIN\'/$$relative_path($$GCS_QT_LIBRARY_PATH, $$GCS_LIBRARY_PATH)
        include(rpath.pri)

        target.path = /$$GCS_LIBRARY_BASENAME/openpilotgcs
    }
    INSTALLS += target
}
