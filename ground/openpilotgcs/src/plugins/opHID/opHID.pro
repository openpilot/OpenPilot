TEMPLATE = lib
TARGET = opHID
include(../../openpilotgcsplugin.pri)
include(opHID_dependencies.pri)
HEADERS += inc/opHID_global.h \
           inc/opHID_plugin.h \
           inc/opHID.h \
           inc/opHID_hidapi.h \
           inc/opHID_const.h \
           inc/opHID_usbmon.h \
           inc/opHID_usbsignal.h \
           inc/hidapi.h
SOURCES += src/opHID_plugin.cpp \
           src/opHID.cpp \
           src/opHID_usbsignal.cpp \
           src/opHID_hidapi.cpp
FORMS += 
RESOURCES += 
DEFINES += OPHID_LIBRARY
OTHER_FILES += opHID.pluginspec
INCLUDEPATH += ./inc
# Platform Specific
win32 { 
    SOURCES += src/opHID_usbmon_win.cpp \
               hidapi/windows/hid.c
    LIBS += -lhid -lsetupapi
}
macx { 
    SOURCES += src/opHID_usbmon_mac.cpp \
               hidapi/mac/hid.c
    SDK = /Developer/SDKs/MacOSX10.5.sdk
    ARCH = -mmacosx-version-min=10.5 \
           -arch \
           ppc \
           -arch \
           i386
    LIBS += $(ARCH) \
            -Wl,-syslibroot,$(SDK) \
            -framework \
            IOKit \
            -framework \
            CoreFoundation
}
linux-g++ {
    SOURCES += src/opHID_usbmon_linux.cpp
    LIBS += -lusb -ludev

# hidapi library
## rawhid
#    SOURCES += hidapi/linux/hid.c
## libusb
    SOURCES += hidapi/libusb/hid.c
    LIBS += `pkg-config libusb-1.0 --libs` -lrt -lpthread
    INCLUDEPATH += /usr/include/libusb-1.0 
#    INCLUDEPATH += `pkg-config libusb-1.0 --cflags`
!exists(/usr/include/libusb-1.0) {
    error(Install libusb-1.0.0-dev using your package manager.)
 }
}
linux-g++-64 {
    SOURCES += src/opHID_usbmon_linux.cpp
    LIBS += -lusb -ludev

# hidapi library
## rawhid
#    SOURCES += hidapi/linux/hid.c
## libusb
    SOURCES += hidapi/libusb/hid.c
    LIBS += `pkg-config libusb-1.0 --libs` -lrt -lpthread
    INCLUDEPATH += /usr/include/libusb-1.0
#    INCLUDEPATH += `pkg-config libusb-1.0 --cflags`
!exists(/usr/include/libusb-1.0) {
    error(Install libusb-1.0.0-dev using your package manager.)
 }
}

