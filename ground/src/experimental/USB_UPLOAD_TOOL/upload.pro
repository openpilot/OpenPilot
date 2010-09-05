#-------------------------------------------------
#
# Project created by QtCreator 2010-07-24T11:26:38
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = OPUploadTool
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    op_dfu.cpp
HEADERS += ../../plugins/rawhid/pjrc_rawhid.h \
    op_dfu.h

win32 {
    SOURCES += ../../plugins/rawhid/pjrc_rawhid_win.cpp
    LIBS += -lhid \
        -lsetupapi
}

macx {
    SOURCES += ../../plugins/rawhid/pjrc_rawhid_mac.cpp
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
    SOURCES += ../../plugins/rawhid/pjrc_rawhid_unix.cpp
    LIBS += -lusb
}
linux-g++-64 {
    SOURCES += ../../plugins/rawhid/pjrc_rawhid_unix.cpp
    LIBS += -lusb
}
