#
# Qmake project for UAVObjGenerator.
# Copyright (c) 2010-2013, The OpenPilot Team, http://www.openpilot.org
#

QT += xml
QT -= gui
macx {
    QMAKE_CXXFLAGS  += -fpermissive
}
TARGET = uavobjgenerator
CONFIG += console
CONFIG -= app_bundle
TEMPLATE = app
SOURCES += main.cpp \
    uavobjectparser.cpp \
    generators/generator_io.cpp \
    generators/java/uavobjectgeneratorjava.cpp \
    generators/flight/uavobjectgeneratorflight.cpp \
    generators/gcs/uavobjectgeneratorgcs.cpp \
    generators/matlab/uavobjectgeneratormatlab.cpp \
    generators/python/uavobjectgeneratorpython.cpp \
    generators/json/uavobjectgeneratorjson.cpp \
    generators/wireshark/uavobjectgeneratorwireshark.cpp \
    generators/generator_common.cpp
HEADERS += uavobjectparser.h \
    generators/generator_io.h \
    generators/java/uavobjectgeneratorjava.h \
    generators/gcs/uavobjectgeneratorgcs.h \
    generators/matlab/uavobjectgeneratormatlab.h \
    generators/python/uavobjectgeneratorpython.h \
    generators/json/uavobjectgeneratorjson.h \
    generators/wireshark/uavobjectgeneratorwireshark.h \
    generators/generator_common.h
