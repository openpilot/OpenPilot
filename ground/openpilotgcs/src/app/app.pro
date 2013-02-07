include(../../openpilotgcs.pri)
include(../shared/qtsingleapplication/qtsingleapplication.pri)
include(gcsversioninfo.pri)

TEMPLATE = app
TARGET = $$GCS_APP_TARGET
DESTDIR = $$GCS_APP_PATH
QT += xml
SOURCES += main.cpp \
    gcssplashscreen.cpp
include(../rpath.pri)
include(../libs/utils/utils.pri)

LIBS *= -l$$qtLibraryName(ExtensionSystem) -l$$qtLibraryName(Aggregation)

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
} else {
    target.path  = /bin
    INSTALLS    += target
}

OTHER_FILES += openpilotgcs.rc

RESOURCES += \
    appresources.qrc

HEADERS += \
    gcssplashscreen.h
