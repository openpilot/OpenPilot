#
# Qmake project for UAVObjGenerator.
# Copyright (c) 2010-2013, The OpenPilot Team, http://www.openpilot.org
#

QT += xml
QT -= gui
macx {
    QMAKE_CXXFLAGS  += -fpermissive
}
TARGET = loggerextractor
CONFIG += console
CONFIG -= app_bundle
TEMPLATE = app
SOURCES += main.cpp \
    uavobjectparser.cpp \
    ../uavobjgenerator/generators/generator_io.cpp 
HEADERS += uavobjectparser.h \
     ../uavobjgenerator/generators/generator_io.h 
