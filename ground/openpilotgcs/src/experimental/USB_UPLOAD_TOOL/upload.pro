#-------------------------------------------------
#
# Project created by QtCreator 2010-07-24T11:26:38
#
#-------------------------------------------------

QT       += core

QT       -= gui
DEFINES                 += QEXTSERIALPORT_LIBRARY
TARGET = OPUploadTool
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app
DEFINES += RAWHID_LIBRARY
SOURCES += main.cpp \
    op_dfu.cpp \
    delay.cpp \
    ./SSP/qssp.cpp \
    ./SSP/port.cpp \
    ./SSP/qsspt.cpp \
../../libs/qextserialport/src/qextserialport.cpp
HEADERS += ../../plugins/rawhid/pjrc_rawhid.h \
 ../../plugins/rawhid/rawhid_global.h \
    op_dfu.h \
    delay.h \
 ../../libs/qextserialport/src/qextserialport.h \
                           ../../libs/qextserialport/src/qextserialenumerator.h \
                           ../../libs/qextserialport/src/qextserialport_global.h \
    ./SSP/qssp.h \
    ./SSP/port.h \
    ./SSP/common.h \
    ./SSP/qsspt.h

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
unix:SOURCES           += ../../libs/qextserialport/src/posix_qextserialport.cpp
unix:!macx:SOURCES     += ../../libs/qextserialport/src/qextserialenumerator_unix.cpp
macx {
  SOURCES          += ../../libs/qextserialport/src/qextserialenumerator_osx.cpp
  LIBS             += -framework IOKit -framework CoreFoundation
}

win32 {
  SOURCES          += ../../libs/qextserialport/src/win_qextserialport.cpp \
../../libs/qextserialport/src/qextserialenumerator_win.cpp
  DEFINES          += WINVER=0x0501 # needed for mingw to pull in appropriate dbt business...probably a better way to do this
  LIBS             += -lsetupapi
}
