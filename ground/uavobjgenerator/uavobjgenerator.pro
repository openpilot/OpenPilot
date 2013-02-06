#
# Qmake project for UAVObjGenerator.
# Copyright (c) 2010-2013, The OpenPilot Team, http://www.openpilot.org
#

TARGET = uavobjgenerator
TEMPLATE = app

QT += xml
QT -= gui

CONFIG += console
CONFIG -= app_bundle

macx {
    QMAKE_CXXFLAGS += -fpermissive
}

win32 {
    # Fix ((packed)) pragma handling issue introduced when upgrading MinGW from 4.4 to 4.8
    # See http://gcc.gnu.org/bugzilla/show_bug.cgi?id=52991
    # The ((packet)) pragma is used in uav metadata struct and other places
    QMAKE_CXXFLAGS += -mno-ms-bitfields
}

SOURCES += main.cpp \
    uavobjectparser.cpp \
    generators/generator_io.cpp \
    generators/java/uavobjectgeneratorjava.cpp \
    generators/flight/uavobjectgeneratorflight.cpp \
    generators/gcs/uavobjectgeneratorgcs.cpp \
    generators/matlab/uavobjectgeneratormatlab.cpp \
    generators/python/uavobjectgeneratorpython.cpp \
    generators/wireshark/uavobjectgeneratorwireshark.cpp \
    generators/generator_common.cpp

HEADERS += uavobjectparser.h \
    generators/generator_io.h \
    generators/java/uavobjectgeneratorjava.h \
    generators/gcs/uavobjectgeneratorgcs.h \
    generators/matlab/uavobjectgeneratormatlab.h \
    generators/python/uavobjectgeneratorpython.h \
    generators/wireshark/uavobjectgeneratorwireshark.h \
    generators/generator_common.h
