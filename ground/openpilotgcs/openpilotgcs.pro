#version check qt
contains(QT_VERSION, ^4\\.[0-5]\\..*) {
    message("Cannot build OpenPilot GCS with Qt version $${QT_VERSION}.")
    error("Cannot build OpenPilot GCS with Qt version $${QT_VERSION}. Use at least Qt 4.6!")
}

include(openpilotgcs.pri)

TEMPLATE  = subdirs
CONFIG   += ordered

#QMAKE_INCDIR_OPENGL += $$ANDROID_PLATFORM_PATH/include
#QMAKE_INCDIR_EGL += $$ANDROID_PLATFORM_PATH/include
#QMAKE_INCDIR_OPENGL_ES2 += $$ANDROID_PLATFORM_PATH/include

SUBDIRS = src share copydata
unix:!macx:!isEmpty(copydata):SUBDIRS += bin

copydata.file = copydata.pro
