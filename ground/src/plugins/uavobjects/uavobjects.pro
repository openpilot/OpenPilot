TEMPLATE = lib
TARGET = UAVObjects
DEFINES += UAVOBJECTS_LIBRARY
include(../../openpilotgcsplugin.pri)
include(uavobjects_dependencies.pri)
HEADERS += uavobjects_global.h \
    uavobject.h \
    uavmetaobject.h \
    uavobjectmanager.h \
    uavdataobject.h \
    uavobjectfield.h \
    uavobjectsinit.h \
    uavobjectsplugin.h \
    examplesettings.h \
    ahrsstatus.h \
    ahrscalibration.h \
    baroaltitude.h \
    attitudeactual.h \
    attitudesettings.h \
    exampleobject2.h \
    exampleobject1.h \
    gcstelemetrystats.h \
    attituderaw.h \
    flighttelemetrystats.h \
    systemstats.h \
    systemalarms.h \
    objectpersistence.h \
    telemetrysettings.h \
    systemsettings.h \
    stabilizationsettings.h \
    flightsituationactual.h \
    manualcontrolsettings.h \
    manualcontrolcommand.h \
    attitudedesired.h \
    actuatorsettings.h \
    actuatordesired.h \
    actuatorcommand.h \
    navigationsettings.h \
    navigationdesired.h \
    gpsposition.h \
    positionactual.h \
    flightbatterystate.h \
    homelocation.h
SOURCES += uavobject.cpp \
    uavmetaobject.cpp \
    uavobjectmanager.cpp \
    uavdataobject.cpp \
    uavobjectfield.cpp \
    uavobjectsinit.cpp \
    uavobjectsplugin.cpp \
    ahrsstatus.cpp \
    ahrscalibration.cpp \
    baroaltitude.cpp \
    attitudeactual.cpp \
    attitudesettings.cpp \
    examplesettings.cpp \
    exampleobject2.cpp \
    exampleobject1.cpp \
    gcstelemetrystats.cpp \
    attituderaw.cpp \
    flighttelemetrystats.cpp \
    systemstats.cpp \
    systemalarms.cpp \
    objectpersistence.cpp \
    telemetrysettings.cpp \
    systemsettings.cpp \
    stabilizationsettings.cpp \
    flightsituationactual.cpp \
    manualcontrolsettings.cpp \
    manualcontrolcommand.cpp \
    attitudedesired.cpp \
    actuatorsettings.cpp \
    actuatordesired.cpp \
    actuatorcommand.cpp \
    navigationsettings.cpp \
    navigationdesired.cpp \
    gpsposition.cpp \
    positionactual.cpp \
    flightbatterystate.cpp \
    homelocation.cpp
OTHER_FILES += UAVObjects.pluginspec
