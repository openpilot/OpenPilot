include(../../openpilotgcs.pri)
include(../shared/qtsingleapplication/qtsingleapplication.pri)

TEMPLATE = app
TARGET = $$GCS_APP_TARGET
DESTDIR = $$GCS_APP_PATH
QT += xml

SOURCES += main.cpp

include(../rpath.pri)
include(../libs/utils/utils.pri)

win32 {
    CONFIG(debug, debug|release):LIBS *= -lExtensionSystemd -lAggregationd  -lQExtSerialPortd
    else:LIBS *= -lExtensionSystem -lAggregation -lQExtSerialPort

    RC_FILE = openpilotgcs.rc
    target.path = /bin
    INSTALLS += target
} else:macx {
    CONFIG(debug, debug|release):LIBS *= -lExtensionSystem_debug -lAggregation_debug
    else:LIBS *= -lExtensionSystem -lAggregation
    LIBS += -framework CoreFoundation
    ICON = openpilotgcs.icns
    QMAKE_INFO_PLIST = Info.plist
    FILETYPES.files = profile.icns prifile.icns
    FILETYPES.path = Contents/Resources
    QMAKE_BUNDLE_DATA += FILETYPES
} else {
    LIBS *= -lExtensionSystem -lAggregation

    target.path  = /bin
    INSTALLS    += target
}

OTHER_FILES += openpilotgcs.rc
