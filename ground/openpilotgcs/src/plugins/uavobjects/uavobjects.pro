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
HEADERS += $$UAVOBJECT_SYNTHETICS/accessorydesired.h \
    $$UAVOBJECT_SYNTHETICS/baroaltitude.h \
    $$UAVOBJECT_SYNTHETICS/baroairspeed.h \
    $$UAVOBJECT_SYNTHETICS/airspeedsettings.h \
    $$UAVOBJECT_SYNTHETICS/airspeedactual.h \
    $$UAVOBJECT_SYNTHETICS/attitudeactual.h \
    $$UAVOBJECT_SYNTHETICS/attitudesimulated.h \
    $$UAVOBJECT_SYNTHETICS/altholdsmoothed.h \
    $$UAVOBJECT_SYNTHETICS/altitudeholddesired.h \
    $$UAVOBJECT_SYNTHETICS/altitudeholdsettings.h \
    $$UAVOBJECT_SYNTHETICS/revocalibration.h \
    $$UAVOBJECT_SYNTHETICS/revosettings.h \
    $$UAVOBJECT_SYNTHETICS/gcstelemetrystats.h \
    $$UAVOBJECT_SYNTHETICS/gyros.h \
    $$UAVOBJECT_SYNTHETICS/gyrosbias.h \
    $$UAVOBJECT_SYNTHETICS/accels.h \
    $$UAVOBJECT_SYNTHETICS/magnetometer.h \
    $$UAVOBJECT_SYNTHETICS/magbias.h \
    $$UAVOBJECT_SYNTHETICS/camerastabsettings.h \
    $$UAVOBJECT_SYNTHETICS/flighttelemetrystats.h \
    $$UAVOBJECT_SYNTHETICS/systemstats.h \
    $$UAVOBJECT_SYNTHETICS/systemalarms.h \
    $$UAVOBJECT_SYNTHETICS/objectpersistence.h \
    $$UAVOBJECT_SYNTHETICS/overosyncstats.h \
    $$UAVOBJECT_SYNTHETICS/overosyncsettings.h \
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
    $$UAVOBJECT_SYNTHETICS/pathaction.h \
    $$UAVOBJECT_SYNTHETICS/pathdesired.h \
    $$UAVOBJECT_SYNTHETICS/pathstatus.h \
    $$UAVOBJECT_SYNTHETICS/gpsvelocity.h \
    $$UAVOBJECT_SYNTHETICS/positionactual.h \
    $$UAVOBJECT_SYNTHETICS/flightbatterystate.h \
    $$UAVOBJECT_SYNTHETICS/homelocation.h \
    $$UAVOBJECT_SYNTHETICS/mixersettings.h \
    $$UAVOBJECT_SYNTHETICS/mixerstatus.h \
    $$UAVOBJECT_SYNTHETICS/velocitydesired.h \
    $$UAVOBJECT_SYNTHETICS/velocityactual.h \
    $$UAVOBJECT_SYNTHETICS/groundtruth.h \
    $$UAVOBJECT_SYNTHETICS/fixedwingpathfollowersettings.h \
    $$UAVOBJECT_SYNTHETICS/fixedwingpathfollowerstatus.h \
    $$UAVOBJECT_SYNTHETICS/vtolpathfollowersettings.h \
    $$UAVOBJECT_SYNTHETICS/relaytuning.h \
    $$UAVOBJECT_SYNTHETICS/relaytuningsettings.h \
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
    $$UAVOBJECT_SYNTHETICS/hwsettings.h \
    $$UAVOBJECT_SYNTHETICS/gcsreceiver.h \
    $$UAVOBJECT_SYNTHETICS/receiveractivity.h \
    $$UAVOBJECT_SYNTHETICS/attitudesettings.h \
    $$UAVOBJECT_SYNTHETICS/txpidsettings.h \
    $$UAVOBJECT_SYNTHETICS/cameradesired.h \
    $$UAVOBJECT_SYNTHETICS/faultsettings.h \
    $$UAVOBJECT_SYNTHETICS/poilearnsettings.h \
    $$UAVOBJECT_SYNTHETICS/poilocation.h \
    $$UAVOBJECT_SYNTHETICS/pipxsettings.h \
    $$UAVOBJECT_SYNTHETICS/pipxstatus.h \
    $$UAVOBJECT_SYNTHETICS/osdsettings.h \
    $$UAVOBJECT_SYNTHETICS/waypoint.h \
    $$UAVOBJECT_SYNTHETICS/waypointactive.h

SOURCES += $$UAVOBJECT_SYNTHETICS/accessorydesired.cpp \
    $$UAVOBJECT_SYNTHETICS/baroaltitude.cpp \
    $$UAVOBJECT_SYNTHETICS/baroairspeed.cpp \
    $$UAVOBJECT_SYNTHETICS/airspeedsettings.cpp \
    $$UAVOBJECT_SYNTHETICS/airspeedactual.cpp \
    $$UAVOBJECT_SYNTHETICS/attitudeactual.cpp \
    $$UAVOBJECT_SYNTHETICS/attitudesimulated.cpp \
    $$UAVOBJECT_SYNTHETICS/altholdsmoothed.cpp \
    $$UAVOBJECT_SYNTHETICS/altitudeholddesired.cpp \
    $$UAVOBJECT_SYNTHETICS/altitudeholdsettings.cpp \
    $$UAVOBJECT_SYNTHETICS/revocalibration.cpp \
    $$UAVOBJECT_SYNTHETICS/revosettings.cpp \
    $$UAVOBJECT_SYNTHETICS/gcstelemetrystats.cpp \
    $$UAVOBJECT_SYNTHETICS/accels.cpp \
    $$UAVOBJECT_SYNTHETICS/gyros.cpp \
    $$UAVOBJECT_SYNTHETICS/gyrosbias.cpp \
    $$UAVOBJECT_SYNTHETICS/magnetometer.cpp \
    $$UAVOBJECT_SYNTHETICS/magbias.cpp \
    $$UAVOBJECT_SYNTHETICS/camerastabsettings.cpp \
    $$UAVOBJECT_SYNTHETICS/flighttelemetrystats.cpp \
    $$UAVOBJECT_SYNTHETICS/systemstats.cpp \
    $$UAVOBJECT_SYNTHETICS/systemalarms.cpp \
    $$UAVOBJECT_SYNTHETICS/objectpersistence.cpp \
    $$UAVOBJECT_SYNTHETICS/overosyncstats.cpp \
    $$UAVOBJECT_SYNTHETICS/overosyncsettings.cpp \
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
    $$UAVOBJECT_SYNTHETICS/pathaction.cpp \
    $$UAVOBJECT_SYNTHETICS/pathdesired.cpp \
    $$UAVOBJECT_SYNTHETICS/pathstatus.cpp \
    $$UAVOBJECT_SYNTHETICS/gpsvelocity.cpp \
    $$UAVOBJECT_SYNTHETICS/positionactual.cpp \
    $$UAVOBJECT_SYNTHETICS/flightbatterystate.cpp \
    $$UAVOBJECT_SYNTHETICS/homelocation.cpp \
    $$UAVOBJECT_SYNTHETICS/mixersettings.cpp \
    $$UAVOBJECT_SYNTHETICS/mixerstatus.cpp \
    $$UAVOBJECT_SYNTHETICS/velocitydesired.cpp \
    $$UAVOBJECT_SYNTHETICS/velocityactual.cpp \
    $$UAVOBJECT_SYNTHETICS/groundtruth.cpp \
    $$UAVOBJECT_SYNTHETICS/fixedwingpathfollowersettings.cpp \
    $$UAVOBJECT_SYNTHETICS/fixedwingpathfollowerstatus.cpp \
    $$UAVOBJECT_SYNTHETICS/vtolpathfollowersettings.cpp \
    $$UAVOBJECT_SYNTHETICS/relaytuningsettings.cpp \
    $$UAVOBJECT_SYNTHETICS/relaytuning.cpp \
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
    $$UAVOBJECT_SYNTHETICS/hwsettings.cpp \
    $$UAVOBJECT_SYNTHETICS/gcsreceiver.cpp \
    $$UAVOBJECT_SYNTHETICS/receiveractivity.cpp \
    $$UAVOBJECT_SYNTHETICS/attitudesettings.cpp \
    $$UAVOBJECT_SYNTHETICS/txpidsettings.cpp \
    $$UAVOBJECT_SYNTHETICS/cameradesired.cpp \
    $$UAVOBJECT_SYNTHETICS/faultsettings.cpp \
    $$UAVOBJECT_SYNTHETICS/poilearnsettings.cpp \
    $$UAVOBJECT_SYNTHETICS/poilocation.cpp \
    $$UAVOBJECT_SYNTHETICS/pipxsettings.cpp \
    $$UAVOBJECT_SYNTHETICS/pipxstatus.cpp \
    $$UAVOBJECT_SYNTHETICS/osdsettings.cpp \
    $$UAVOBJECT_SYNTHETICS/waypoint.cpp \
    $$UAVOBJECT_SYNTHETICS/waypointactive.cpp
