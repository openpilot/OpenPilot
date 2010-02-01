#version check qt
contains(QT_VERSION, ^4\.[0-5]\..*) {
    message("Cannot build OpenPilot GCS with Qt version $${QT_VERSION}.")
    error("Use at least Qt 4.6.")
}

include(openpilotgcs.pri)

TEMPLATE  = subdirs
CONFIG   += ordered

SUBDIRS = src share
unix:!macx:!isEmpty(copydata):SUBDIRS += bin
