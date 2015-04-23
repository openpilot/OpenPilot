TEMPLATE = lib
TARGET = UAVObjects

DEFINES += UAVOBJECTS_LIBRARY

include(../../openpilotgcsplugin.pri)
include(uavobjects_dependencies.pri)

HEADERS += \
    uavobjects_global.h \
    uavobject.h \
    uavmetaobject.h \
    uavobjectmanager.h \
    uavdataobject.h \
    uavobjectfield.h \
    uavobjectsinit.h \
    uavobjectsplugin.h
SOURCES += \
    uavobject.cpp \
    uavmetaobject.cpp \
    uavobjectmanager.cpp \
    uavdataobject.cpp \
    uavobjectfield.cpp \
    uavobjectsplugin.cpp

OTHER_FILES += UAVObjects.pluginspec

UAVOBJ_XML_DIR = $${ROOT_DIR}/shared/uavobjectdefinition
UAVOBJ_ROOT_DIR = $${ROOT_DIR}

CONFIG(debug, debug|release) {
    BUILD_CONF = debug
} else {
    BUILD_CONF = release
}

win32 {
    UAVOBJGENERATOR = ../../../../uavobjgenerator/$${BUILD_CONF}/uavobjgenerator.exe
} else {
    UAVOBJGENERATOR = ../../../../uavobjgenerator/uavobjgenerator
}

# Add in all of the uavobjects
UAVOBJS = \
    $${UAVOBJ_XML_DIR}/vtolselftuningstats.xml \
    $${UAVOBJ_XML_DIR}/accelgyrosettings.xml \
    $${UAVOBJ_XML_DIR}/accessorydesired.xml \
    $${UAVOBJ_XML_DIR}/barosensor.xml \
    $${UAVOBJ_XML_DIR}/airspeedsensor.xml \
    $${UAVOBJ_XML_DIR}/airspeedsettings.xml \
    $${UAVOBJ_XML_DIR}/airspeedstate.xml \
    $${UAVOBJ_XML_DIR}/attitudestate.xml \
    $${UAVOBJ_XML_DIR}/attitudesimulated.xml \
    $${UAVOBJ_XML_DIR}/altitudeholdsettings.xml \
    $${UAVOBJ_XML_DIR}/altitudeholdstatus.xml \
    $${UAVOBJ_XML_DIR}/altitudefiltersettings.xml \
    $${UAVOBJ_XML_DIR}/debuglogsettings.xml \
    $${UAVOBJ_XML_DIR}/debuglogcontrol.xml \
    $${UAVOBJ_XML_DIR}/debuglogstatus.xml \
    $${UAVOBJ_XML_DIR}/debuglogentry.xml \
    $${UAVOBJ_XML_DIR}/ekfconfiguration.xml \
    $${UAVOBJ_XML_DIR}/ekfstatevariance.xml \
    $${UAVOBJ_XML_DIR}/revocalibration.xml \
    $${UAVOBJ_XML_DIR}/revosettings.xml \
    $${UAVOBJ_XML_DIR}/gcstelemetrystats.xml \
    $${UAVOBJ_XML_DIR}/gyrostate.xml \
    $${UAVOBJ_XML_DIR}/gyrosensor.xml \
    $${UAVOBJ_XML_DIR}/accelsensor.xml \
    $${UAVOBJ_XML_DIR}/accelstate.xml \
    $${UAVOBJ_XML_DIR}/magsensor.xml \
    $${UAVOBJ_XML_DIR}/magstate.xml \
    $${UAVOBJ_XML_DIR}/camerastabsettings.xml \
    $${UAVOBJ_XML_DIR}/flighttelemetrystats.xml \
    $${UAVOBJ_XML_DIR}/systemstats.xml \
    $${UAVOBJ_XML_DIR}/systemalarms.xml \
    $${UAVOBJ_XML_DIR}/objectpersistence.xml \
    $${UAVOBJ_XML_DIR}/overosyncstats.xml \
    $${UAVOBJ_XML_DIR}/overosyncsettings.xml \
    $${UAVOBJ_XML_DIR}/systemsettings.xml \
    $${UAVOBJ_XML_DIR}/stabilizationstatus.xml \
    $${UAVOBJ_XML_DIR}/stabilizationsettings.xml \
    $${UAVOBJ_XML_DIR}/stabilizationsettingsbank1.xml \
    $${UAVOBJ_XML_DIR}/stabilizationsettingsbank2.xml \
    $${UAVOBJ_XML_DIR}/stabilizationsettingsbank3.xml \
    $${UAVOBJ_XML_DIR}/stabilizationbank.xml \
    $${UAVOBJ_XML_DIR}/manualcontrolsettings.xml \
    $${UAVOBJ_XML_DIR}/manualcontrolcommand.xml \
    $${UAVOBJ_XML_DIR}/flightmodesettings.xml \
    $${UAVOBJ_XML_DIR}/stabilizationdesired.xml \
    $${UAVOBJ_XML_DIR}/actuatorsettings.xml \
    $${UAVOBJ_XML_DIR}/actuatordesired.xml \
    $${UAVOBJ_XML_DIR}/actuatorcommand.xml \
    $${UAVOBJ_XML_DIR}/gpspositionsensor.xml \
    $${UAVOBJ_XML_DIR}/gpstime.xml \
    $${UAVOBJ_XML_DIR}/gpssatellites.xml \
    $${UAVOBJ_XML_DIR}/gpssettings.xml \
    $${UAVOBJ_XML_DIR}/pathaction.xml \
    $${UAVOBJ_XML_DIR}/pathdesired.xml \
    $${UAVOBJ_XML_DIR}/pathplan.xml \
    $${UAVOBJ_XML_DIR}/pathstatus.xml \
    $${UAVOBJ_XML_DIR}/pathsummary.xml \
    $${UAVOBJ_XML_DIR}/gpsvelocitysensor.xml \
    $${UAVOBJ_XML_DIR}/positionstate.xml \
    $${UAVOBJ_XML_DIR}/flightbatterystate.xml \
    $${UAVOBJ_XML_DIR}/homelocation.xml \
    $${UAVOBJ_XML_DIR}/mixersettings.xml \
    $${UAVOBJ_XML_DIR}/mixerstatus.xml \
    $${UAVOBJ_XML_DIR}/velocitydesired.xml \
    $${UAVOBJ_XML_DIR}/velocitystate.xml \
    $${UAVOBJ_XML_DIR}/groundtruth.xml \
    $${UAVOBJ_XML_DIR}/fixedwingpathfollowersettings.xml \
    $${UAVOBJ_XML_DIR}/fixedwingpathfollowerstatus.xml \
    $${UAVOBJ_XML_DIR}/vtolpathfollowersettings.xml \
    $${UAVOBJ_XML_DIR}/ratedesired.xml \
    $${UAVOBJ_XML_DIR}/firmwareiapobj.xml \
    $${UAVOBJ_XML_DIR}/i2cstats.xml \
    $${UAVOBJ_XML_DIR}/flightbatterysettings.xml \
    $${UAVOBJ_XML_DIR}/taskinfo.xml \
    $${UAVOBJ_XML_DIR}/callbackinfo.xml \
    $${UAVOBJ_XML_DIR}/flightplanstatus.xml \
    $${UAVOBJ_XML_DIR}/flightplansettings.xml \
    $${UAVOBJ_XML_DIR}/flightplancontrol.xml \
    $${UAVOBJ_XML_DIR}/watchdogstatus.xml \
    $${UAVOBJ_XML_DIR}/nedaccel.xml \
    $${UAVOBJ_XML_DIR}/sonaraltitude.xml \
    $${UAVOBJ_XML_DIR}/flightstatus.xml \
    $${UAVOBJ_XML_DIR}/hwsettings.xml \
    $${UAVOBJ_XML_DIR}/gcsreceiver.xml \
    $${UAVOBJ_XML_DIR}/receiveractivity.xml \
    $${UAVOBJ_XML_DIR}/attitudesettings.xml \
    $${UAVOBJ_XML_DIR}/txpidsettings.xml \
    $${UAVOBJ_XML_DIR}/cameradesired.xml \
    $${UAVOBJ_XML_DIR}/faultsettings.xml \
    $${UAVOBJ_XML_DIR}/poilearnsettings.xml \
    $${UAVOBJ_XML_DIR}/poilocation.xml \
    $${UAVOBJ_XML_DIR}/oplinksettings.xml \
    $${UAVOBJ_XML_DIR}/oplinkstatus.xml \
    $${UAVOBJ_XML_DIR}/oplinkreceiver.xml \
    $${UAVOBJ_XML_DIR}/radiocombridgestats.xml \
    $${UAVOBJ_XML_DIR}/osdsettings.xml \
    $${UAVOBJ_XML_DIR}/waypoint.xml \
    $${UAVOBJ_XML_DIR}/waypointactive.xml \
    $${UAVOBJ_XML_DIR}/mpu6000settings.xml \
    $${UAVOBJ_XML_DIR}/takeofflocation.xml \
    $${UAVOBJ_XML_DIR}/auxmagsensor.xml \
    $${UAVOBJ_XML_DIR}/auxmagsettings.xml \
    $${UAVOBJ_XML_DIR}/gpsextendedstatus.xml \
    $${UAVOBJ_XML_DIR}/perfcounter.xml

include(uavobjgenerator.pri)
