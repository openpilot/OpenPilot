include(../openpilotgcs.pri)

win32 {
    DLLDESTDIR = $$GCS_APP_PATH
}

DESTDIR = $$GCS_LIBRARY_PATH

include(rpath.pri)

TARGET = $$qtLibraryTarget($$TARGET)

contains(QT_CONFIG, reduce_exports):CONFIG += hide_symbols

!macx {
    win32 {
        target.path = /bin
        target.files = $$DESTDIR/$${TARGET}.dll
    } else {
        target.path = /$$GCS_LIBRARY_BASENAME/openpilotgcs
    }
    INSTALLS += target
}
