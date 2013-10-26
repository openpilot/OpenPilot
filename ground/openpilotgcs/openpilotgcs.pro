#
# Qmake project for the OpenPilot GCS.
# Copyright (c) 2009-2013, The OpenPilot Team, http://www.openpilot.org
#

#version check qt
contains(QT_VERSION, ^4\\.[0-7]\\..*) {
    message("Cannot build OpenPilot GCS with Qt version $${QT_VERSION}.")
    error("Cannot build OpenPilot GCS with Qt version $${QT_VERSION}. Use at least Qt 4.8!")
}
cache()
macx {
    # This ensures that code is compiled with the /usr/bin version of gcc instead
    # of the gcc in XCode.app/Context/Development
    QMAKE_CC = /usr/bin/gcc
    QMAKE_CXX = /usr/bin/g++ 
    QMAKE_LINK = /usr/bin/g++
}

include(openpilotgcs.pri)

TEMPLATE  = subdirs
CONFIG   += ordered

DEFINES += USE_PATHPLANNER

SUBDIRS = src share copydata
unix:!macx:!isEmpty(copydata):SUBDIRS += bin

copydata.file = copydata.pro
