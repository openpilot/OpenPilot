#
# Qmake project for the OpenPilot GCS.
# Copyright (c) 2009-2013, The OpenPilot Team, http://www.openpilot.org
#

#version check qt
contains(QT_VERSION, ^4\\.[0-7]\\..*) {
    message("Cannot build OpenPilot GCS with Qt version $${QT_VERSION}.")
    error("Cannot build OpenPilot GCS with Qt version $${QT_VERSION}. Use at least Qt 4.8!")
}

include(openpilotgcs.pri)

TEMPLATE  = subdirs
CONFIG   += ordered

DEFINES += USE_PATHPLANNER

SUBDIRS = src share copydata
unix:!macx:!isEmpty(copydata):SUBDIRS += bin

copydata.file = copydata.pro
