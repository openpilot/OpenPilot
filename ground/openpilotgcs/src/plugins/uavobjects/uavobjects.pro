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
    uavobjectsplugin.h

SOURCES += uavobject.cpp \
    uavmetaobject.cpp \
    uavobjectmanager.cpp \
    uavdataobject.cpp \
    uavobjectfield.cpp \
    uavobjectsplugin.cpp

OTHER_FILES += UAVObjects.pluginspec

# Add in all of the synthetic/generated uavobject files
HEADERS += $$UAVOBJECT_SYNTHETICS/ahrsstatus.h \
    $$UAVOBJECT_SYNTHETICS/ahrscalibration.h \
    $$UAVOBJECT_SYNTHETICS/baroaltitude.h \
    $$UAVOBJECT_SYNTHETICS/attitudeactual.h \
    $$UAVOBJECT_SYNTHETICS/ahrssettings.h \
    $$UAVOBJECT_SYNTHETICS/gcstelemetrystats.h \
    $$UAVOBJECT_SYNTHETICS/attituderaw.h \
    $$UAVOBJECT_SYNTHETICS/flighttelemetrystats.h \
    $$UAVOBJECT_SYNTHETICS/systemstats.h \
    $$UAVOBJECT_SYNTHETICS/systemalarms.h \
    $$UAVOBJECT_SYNTHETICS/objectpersistence.h \
    $$UAVOBJECT_SYNTHETICS/telemetrysettings.h \
    $$UAVOBJECT_SYNTHETICS/systemsettings.h \
    $$UAVOBJECT_SYNTHETICS/stabilizationsettings.h \
    $$UAVOBJECT_SYNTHETICS/manualcontrolsettings.h \
    $$UAVOBJECT_SYNTHETICS/manualcontrolcommand.h \
    $$UAVOBJECT_SYNTHETICS/stabilizationdesired.h \
    $$UAVOBJECT_SYNTHETICS/actuatorsettings.h \
    $$UAVOBJECT_SYNTHETICS/actuatordesired.h \
    $$UAVOBJECT_SYNTHETICS/actuatorcommand.h \
    $$UAVOBJECT_SYNTHETICS/gpsposition.h \
    $$UAVOBJECT_SYNTHETICS/gpstime.h \
    $$UAVOBJECT_SYNTHETICS/gpssatellites.h \
    $$UAVOBJECT_SYNTHETICS/positionactual.h \
    $$UAVOBJECT_SYNTHETICS/flightbatterystate.h \
    $$UAVOBJECT_SYNTHETICS/homelocation.h \
    $$UAVOBJECT_SYNTHETICS/mixersettings.h \
    $$UAVOBJECT_SYNTHETICS/mixerstatus.h \
    $$UAVOBJECT_SYNTHETICS/velocitydesired.h \
    $$UAVOBJECT_SYNTHETICS/velocityactual.h \
    $$UAVOBJECT_SYNTHETICS/guidancesettings.h \
    $$UAVOBJECT_SYNTHETICS/positiondesired.h \
    $$UAVOBJECT_SYNTHETICS/ratedesired.h \
    $$UAVOBJECT_SYNTHETICS/firmwareiapobj.h \
    $$UAVOBJECT_SYNTHETICS/i2cstats.h \
    $$UAVOBJECT_SYNTHETICS/flightbatterysettings.h \
    $$UAVOBJECT_SYNTHETICS/taskinfo.h \
    $$UAVOBJECT_SYNTHETICS/flightplanstatus.h \
    $$UAVOBJECT_SYNTHETICS/flightplansettings.h \
    $$UAVOBJECT_SYNTHETICS/flightplancontrol.h \
    $$UAVOBJECT_SYNTHETICS/watchdogstatus.h \
    $$UAVOBJECT_SYNTHETICS/nedaccel.h \
    $$UAVOBJECT_SYNTHETICS/sonaraltitude.h \
    $$UAVOBJECT_SYNTHETICS/flightstatus.h \
    $$UAVOBJECT_SYNTHETICS/attitudesettings.h

SOURCES += $$UAVOBJECT_SYNTHETICS/ahrsstatus.cpp \
    $$UAVOBJECT_SYNTHETICS/ahrscalibration.cpp \
    $$UAVOBJECT_SYNTHETICS/baroaltitude.cpp \
    $$UAVOBJECT_SYNTHETICS/attitudeactual.cpp \
    $$UAVOBJECT_SYNTHETICS/ahrssettings.cpp \
    $$UAVOBJECT_SYNTHETICS/gcstelemetrystats.cpp \
    $$UAVOBJECT_SYNTHETICS/attituderaw.cpp \
    $$UAVOBJECT_SYNTHETICS/flighttelemetrystats.cpp \
    $$UAVOBJECT_SYNTHETICS/systemstats.cpp \
    $$UAVOBJECT_SYNTHETICS/systemalarms.cpp \
    $$UAVOBJECT_SYNTHETICS/objectpersistence.cpp \
    $$UAVOBJECT_SYNTHETICS/telemetrysettings.cpp \
    $$UAVOBJECT_SYNTHETICS/systemsettings.cpp \
    $$UAVOBJECT_SYNTHETICS/stabilizationsettings.cpp \
    $$UAVOBJECT_SYNTHETICS/manualcontrolsettings.cpp \
    $$UAVOBJECT_SYNTHETICS/manualcontrolcommand.cpp \
    $$UAVOBJECT_SYNTHETICS/stabilizationdesired.cpp \
    $$UAVOBJECT_SYNTHETICS/actuatorsettings.cpp \
    $$UAVOBJECT_SYNTHETICS/actuatordesired.cpp \
    $$UAVOBJECT_SYNTHETICS/actuatorcommand.cpp \
    $$UAVOBJECT_SYNTHETICS/gpsposition.cpp \
    $$UAVOBJECT_SYNTHETICS/gpstime.cpp \
    $$UAVOBJECT_SYNTHETICS/gpssatellites.cpp \
    $$UAVOBJECT_SYNTHETICS/positionactual.cpp \
    $$UAVOBJECT_SYNTHETICS/flightbatterystate.cpp \
    $$UAVOBJECT_SYNTHETICS/homelocation.cpp \
    $$UAVOBJECT_SYNTHETICS/mixersettings.cpp \
    $$UAVOBJECT_SYNTHETICS/mixerstatus.cpp \
    $$UAVOBJECT_SYNTHETICS/velocitydesired.cpp \
    $$UAVOBJECT_SYNTHETICS/velocityactual.cpp \
    $$UAVOBJECT_SYNTHETICS/guidancesettings.cpp \
    $$UAVOBJECT_SYNTHETICS/positiondesired.cpp \
    $$UAVOBJECT_SYNTHETICS/ratedesired.cpp \
    $$UAVOBJECT_SYNTHETICS/firmwareiapobj.cpp \
    $$UAVOBJECT_SYNTHETICS/i2cstats.cpp \
    $$UAVOBJECT_SYNTHETICS/flightbatterysettings.cpp \
    $$UAVOBJECT_SYNTHETICS/taskinfo.cpp \
    $$UAVOBJECT_SYNTHETICS/flightplanstatus.cpp \
    $$UAVOBJECT_SYNTHETICS/flightplansettings.cpp \
    $$UAVOBJECT_SYNTHETICS/flightplancontrol.cpp \
    $$UAVOBJECT_SYNTHETICS/watchdogstatus.cpp \
    $$UAVOBJECT_SYNTHETICS/nedaccel.cpp \
    $$UAVOBJECT_SYNTHETICS/sonaraltitude.cpp \
    $$UAVOBJECT_SYNTHETICS/uavobjectsinit.cpp \
    $$UAVOBJECT_SYNTHETICS/flightstatus.cpp \
    $$UAVOBJECT_SYNTHETICS/attitudesettings.cpp
