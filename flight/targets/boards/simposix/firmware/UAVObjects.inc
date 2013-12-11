#####
# Project: OpenPilot
#
# Makefile for OpenPilot UAVObject code
#
# The OpenPilot Team, http://www.openpilot.org, Copyright (C) 2011.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
#####

# These are the UAVObjects supposed to be build as part of the OpenPilot target
# (all architectures)

UAVOBJSRCFILENAMES =
UAVOBJSRCFILENAMES += accessorydesired
UAVOBJSRCFILENAMES += actuatorcommand
UAVOBJSRCFILENAMES += actuatordesired
UAVOBJSRCFILENAMES += actuatorsettings
UAVOBJSRCFILENAMES += altholdsmoothed
UAVOBJSRCFILENAMES += attitudesettings
UAVOBJSRCFILENAMES += attitudestate
UAVOBJSRCFILENAMES += attitudesimulated
UAVOBJSRCFILENAMES += gyrostate
UAVOBJSRCFILENAMES += gyrosensor
UAVOBJSRCFILENAMES += accelstate
UAVOBJSRCFILENAMES += accelsensor
UAVOBJSRCFILENAMES += magsensor
UAVOBJSRCFILENAMES += magstate
UAVOBJSRCFILENAMES += barosensor
UAVOBJSRCFILENAMES += airspeedsensor
UAVOBJSRCFILENAMES += airspeedsettings
UAVOBJSRCFILENAMES += airspeedstate
UAVOBJSRCFILENAMES += debuglogsettings
UAVOBJSRCFILENAMES += debuglogcontrol
UAVOBJSRCFILENAMES += debuglogstatus
UAVOBJSRCFILENAMES += debuglogentry
UAVOBJSRCFILENAMES += flightbatterysettings
UAVOBJSRCFILENAMES += firmwareiapobj
UAVOBJSRCFILENAMES += flightbatterystate
UAVOBJSRCFILENAMES += flightplancontrol
UAVOBJSRCFILENAMES += flightplansettings
UAVOBJSRCFILENAMES += flightplanstatus
UAVOBJSRCFILENAMES += flighttelemetrystats
UAVOBJSRCFILENAMES += gcstelemetrystats
UAVOBJSRCFILENAMES += gpspositionsensor
UAVOBJSRCFILENAMES += gpssatellites
UAVOBJSRCFILENAMES += gpstime
UAVOBJSRCFILENAMES += gpsvelocitysensor
UAVOBJSRCFILENAMES += gpssettings
UAVOBJSRCFILENAMES += vtolpathfollowersettings
UAVOBJSRCFILENAMES += fixedwingpathfollowersettings
UAVOBJSRCFILENAMES += fixedwingpathfollowerstatus
UAVOBJSRCFILENAMES += homelocation
UAVOBJSRCFILENAMES += i2cstats
UAVOBJSRCFILENAMES += manualcontrolcommand
UAVOBJSRCFILENAMES += manualcontrolsettings
UAVOBJSRCFILENAMES += mixersettings
UAVOBJSRCFILENAMES += mixerstatus
UAVOBJSRCFILENAMES += nedaccel
UAVOBJSRCFILENAMES += objectpersistence
UAVOBJSRCFILENAMES += overosyncstats
UAVOBJSRCFILENAMES += pathaction
UAVOBJSRCFILENAMES += pathdesired
UAVOBJSRCFILENAMES += pathstatus
UAVOBJSRCFILENAMES += positionstate
UAVOBJSRCFILENAMES += ratedesired
UAVOBJSRCFILENAMES += revocalibration
UAVOBJSRCFILENAMES += relaytuning
UAVOBJSRCFILENAMES += relaytuningsettings
UAVOBJSRCFILENAMES += sonaraltitude
UAVOBJSRCFILENAMES += stabilizationdesired
UAVOBJSRCFILENAMES += stabilizationsettings
UAVOBJSRCFILENAMES += stabilizationbank
UAVOBJSRCFILENAMES += systemalarms
UAVOBJSRCFILENAMES += systemsettings
UAVOBJSRCFILENAMES += systemstats
UAVOBJSRCFILENAMES += taskinfo
UAVOBJSRCFILENAMES += velocitystate
UAVOBJSRCFILENAMES += velocitydesired
UAVOBJSRCFILENAMES += watchdogstatus
UAVOBJSRCFILENAMES += waypoint
UAVOBJSRCFILENAMES += waypointactive
UAVOBJSRCFILENAMES += poilocation
UAVOBJSRCFILENAMES += poilearnsettings
UAVOBJSRCFILENAMES += flightstatus
UAVOBJSRCFILENAMES += hwsettings
UAVOBJSRCFILENAMES += receiveractivity
UAVOBJSRCFILENAMES += cameradesired
UAVOBJSRCFILENAMES += camerastabsettings
UAVOBJSRCFILENAMES += altitudeholdsettings
UAVOBJSRCFILENAMES += altitudefiltersettings
UAVOBJSRCFILENAMES += revosettings
UAVOBJSRCFILENAMES += altitudeholddesired
UAVOBJSRCFILENAMES += ekfconfiguration
UAVOBJSRCFILENAMES += ekfstatevariance

UAVOBJSRC = $(foreach UAVOBJSRCFILE,$(UAVOBJSRCFILENAMES),$(UAVOBJSYNTHDIR)/$(UAVOBJSRCFILE).c )
UAVOBJDEFINE = $(foreach UAVOBJSRCFILE,$(UAVOBJSRCFILENAMES),-DUAVOBJ_INIT_$(UAVOBJSRCFILE) )
