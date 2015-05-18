include(../../openpilotgcs.pri)
include(../shared/qtsingleapplication/qtsingleapplication.pri)

TEMPLATE = app
TARGET = $$GCS_APP_TARGET
DESTDIR = $$GCS_APP_PATH

QT += xml widgets

SOURCES += main.cpp \
    gcssplashscreen.cpp

include(../libs/utils/utils.pri)
include(../libs/version_info/version_info.pri)

LIBS *= -l$$qtLibraryName(ExtensionSystem) -l$$qtLibraryName(Aggregation)

DEFINES += PLUGIN_REL_PATH=$$shell_quote(\"$$relative_path($$GCS_PLUGIN_PATH, $$GCS_APP_PATH)\")
DEFINES += GCS_NAME=$$shell_quote(\"$$GCS_FULL_NAME\")

win32 {
    RC_FILE = openpilotgcs.rc
    target.path = /bin
    INSTALLS += target
} else:macx {
    LIBS += -framework CoreFoundation
    ICON = openpilotgcs.icns
    QMAKE_INFO_PLIST = Info.plist
    FILETYPES.files = profile.icns prifile.icns
    FILETYPES.path = Contents/Resources
    QMAKE_BUNDLE_DATA += FILETYPES
    QMAKE_LFLAGS_SONAME = -Wl,-install_name,@executable_path/../Plugins/
} else {
    target.path  = /bin
    INSTALLS    += target
    QMAKE_RPATHDIR  = $$shell_quote(\$$ORIGIN/$$relative_path($$GCS_LIBRARY_PATH, $$GCS_APP_PATH))
    QMAKE_RPATHDIR += $$shell_quote(\$$ORIGIN/$$relative_path($$GCS_QT_LIBRARY_PATH, $$GCS_APP_PATH))
    include(../rpath.pri)

    equals(copyqt, 1) {
        RESOURCES += qtconf.qrc
    }
}

OTHER_FILES += openpilotgcs.rc

RESOURCES += \
    appresources.qrc

HEADERS += \
    gcssplashscreen.h
