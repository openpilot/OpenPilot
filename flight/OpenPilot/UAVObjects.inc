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
UAVOBJSRCFILENAMES += actuatorcommand
UAVOBJSRCFILENAMES += actuatordesired
UAVOBJSRCFILENAMES += actuatorsettings
UAVOBJSRCFILENAMES += ahrscalibration
UAVOBJSRCFILENAMES += ahrssettings
UAVOBJSRCFILENAMES += ahrsstatus
UAVOBJSRCFILENAMES += attitudeactual
UAVOBJSRCFILENAMES += attituderaw
UAVOBJSRCFILENAMES += baroaltitude
UAVOBJSRCFILENAMES += flightbatterysettings
UAVOBJSRCFILENAMES += firmwareiapobj
UAVOBJSRCFILENAMES += flightbatterystate
UAVOBJSRCFILENAMES += flightplancontrol
UAVOBJSRCFILENAMES += flightplansettings
UAVOBJSRCFILENAMES += flightplanstatus
UAVOBJSRCFILENAMES += flighttelemetrystats
UAVOBJSRCFILENAMES += gcstelemetrystats
UAVOBJSRCFILENAMES += gpsposition
UAVOBJSRCFILENAMES += gpssatellites
UAVOBJSRCFILENAMES += gpstime
UAVOBJSRCFILENAMES += guidancesettings
UAVOBJSRCFILENAMES += homelocation
UAVOBJSRCFILENAMES += i2cstats
UAVOBJSRCFILENAMES += manualcontrolcommand
UAVOBJSRCFILENAMES += manualcontrolsettings
UAVOBJSRCFILENAMES += mixersettings
UAVOBJSRCFILENAMES += mixerstatus
UAVOBJSRCFILENAMES += nedaccel
UAVOBJSRCFILENAMES += objectpersistence
UAVOBJSRCFILENAMES += positionactual
UAVOBJSRCFILENAMES += positiondesired
UAVOBJSRCFILENAMES += ratedesired
UAVOBJSRCFILENAMES += sonaraltitude
UAVOBJSRCFILENAMES += stabilizationdesired
UAVOBJSRCFILENAMES += stabilizationsettings
UAVOBJSRCFILENAMES += systemalarms
UAVOBJSRCFILENAMES += systemsettings
UAVOBJSRCFILENAMES += systemstats
UAVOBJSRCFILENAMES += taskinfo
UAVOBJSRCFILENAMES += telemetrysettings
UAVOBJSRCFILENAMES += velocityactual
UAVOBJSRCFILENAMES += velocitydesired
UAVOBJSRCFILENAMES += watchdogstatus
UAVOBJSRCFILENAMES += flightstatus

UAVOBJSRC = $(foreach UAVOBJSRCFILE,$(UAVOBJSRCFILENAMES),$(UAVOBJSYNTHDIR)/$(UAVOBJSRCFILE).c )
UAVOBJDEFINE = $(foreach UAVOBJSRCFILE,$(UAVOBJSRCFILENAMES),-DUAVOBJ_INIT_$(UAVOBJSRCFILE) )
