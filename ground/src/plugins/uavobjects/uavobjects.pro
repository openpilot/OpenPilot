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
    ahrsstatus.h \
    ahrscalibration.h \
    baroaltitude.h \
    attitudeactual.h \
    ahrssettings.h \
    gcstelemetrystats.h \
    attituderaw.h \
    flighttelemetrystats.h \
    systemstats.h \
    systemalarms.h \
    objectpersistence.h \
    telemetrysettings.h \
    systemsettings.h \
    stabilizationsettings.h \
    manualcontrolsettings.h \
    manualcontrolcommand.h \
    attitudedesired.h \
    actuatorsettings.h \
    actuatordesired.h \
    actuatorcommand.h \
    gpsposition.h \
    gpstime.h \
    gpssatellites.h \
    positionactual.h \
    flightbatterystate.h \
    homelocation.h \
    mixersettings.h \
    mixerstatus.h \
    velocitydesired.h \
    velocityactual.h \
    guidancesettings.h \
    positiondesired.h \
    firmwareiapobj.h

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
    ahrssettings.cpp \
    gcstelemetrystats.cpp \
    attituderaw.cpp \
    flighttelemetrystats.cpp \
    systemstats.cpp \
    systemalarms.cpp \
    objectpersistence.cpp \
    telemetrysettings.cpp \
    systemsettings.cpp \
    stabilizationsettings.cpp \
    manualcontrolsettings.cpp \
    manualcontrolcommand.cpp \
    attitudedesired.cpp \
    actuatorsettings.cpp \
    actuatordesired.cpp \
    actuatorcommand.cpp \
    gpsposition.cpp \
    gpstime.cpp \
    gpssatellites.cpp \
    positionactual.cpp \
    flightbatterystate.cpp \
    homelocation.cpp \
    mixersettings.cpp \
    mixerstatus.cpp \
    velocitydesired.cpp \
    velocityactual.cpp \
    guidancesettings.cpp \
    positiondesired.cpp \
    firmwareiapobj.cpp
OTHER_FILES += UAVObjects.pluginspec
