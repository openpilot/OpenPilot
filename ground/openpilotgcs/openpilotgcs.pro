#version check qt
contains(QT_VERSION, ^4\\.[0-5]\\..*) {
    message("Cannot build OpenPilot GCS with Qt version $${QT_VERSION}.")
    error("Cannot build OpenPilot GCS with Qt version $${QT_VERSION}. Use at least Qt 4.6!")
}

include(openpilotgcs.pri)

TEMPLATE  = subdirs
CONFIG   += ordered

SUBDIRS = src share copydata
unix:!macx:!isEmpty(copydata):SUBDIRS += bin

copydata.file = copydata.pro
