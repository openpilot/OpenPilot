#
# Qmake project for the OpenPilot GCS.
# Copyright (c) 2009-2013, The OpenPilot Team, http://www.openpilot.org
#

cache()

#check Qt version
QT_VERSION = $$[QT_VERSION]
QT_VERSION = $$split(QT_VERSION, ".")
QT_VER_MAJ = $$member(QT_VERSION, 0)
QT_VER_MIN = $$member(QT_VERSION, 1)
 
lessThan(QT_VER_MAJ, 5) | lessThan(QT_VER_MIN, 1) {
   error(OpenPilot GCS requires Qt 5.1.0 or newer but Qt $$[QT_VERSION] was detected.)
}

macx {
    # This ensures that code is compiled with the /usr/bin version of gcc instead
    # of the gcc in XCode.app/Context/Development
    QMAKE_CC = /usr/bin/gcc
    QMAKE_CXX = /usr/bin/g++ 
    QMAKE_LINK = /usr/bin/g++
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.6
}

include(openpilotgcs.pri)

TEMPLATE  = subdirs
CONFIG   += ordered

DEFINES += USE_PATHPLANNER

SUBDIRS = src share copydata
unix:!macx:!isEmpty(copydata):SUBDIRS += bin

copydata.file = copydata.pro
