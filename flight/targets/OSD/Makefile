#
# Copyright (c) 2009-2013, The OpenPilot Team, http://www.openpilot.org
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
#

ifndef OPENPILOT_IS_COOL
    $(error Top level Makefile must be used to build this target)
endif

include $(ROOT_DIR)/make/boards/$(BOARD_NAME)/board-info.mk
include $(ROOT_DIR)/make/firmware-defs.mk

# ARM DSP library
override USE_DSP_LIB := NO

# List of mandatory modules to include
MODULES += Osd/osdgen
MODULES += Osd/osdinput
MODULES += Osd/WavPlayer
MODULES += GPS
MODULES += Extensions/MagBaro
MODULES += FirmwareIAP
MODULES += Telemetry

OPTMODULES =

# Misc options
CFLAGS += -ffast-math

# List C source files here (C dependencies are automatically generated).
# Use file-extension c for "c-only"-files
ifndef TESTAPP
    ## Application Core
    SRC += ${OPMODULEDIR}/System/systemmod.c
    SRC += $(OPSYSTEM)/osd.c
    SRC += $(OPSYSTEM)/pios_board.c
    SRC += $(OPSYSTEM)/pios_usb_board_data.c
    SRC += $(FLIGHTLIB)/alarms.c
    SRC += $(OPUAVTALK)/uavtalk.c
    SRC += $(OPUAVOBJ)/uavobjectmanager.c
    SRC += $(OPUAVOBJ)/eventdispatcher.c

    ## OSD fonts
    SRC += $(OPSYSTEM)/fonts.c
    SRC += $(OPSYSTEM)/font_outlined8x14.c
    SRC += $(OPSYSTEM)/font_outlined8x8.c

    #ifeq ($(DEBUG), YES)
    SRC += $(OPSYSTEM)/dcc_stdio.c
    SRC += $(OPSYSTEM)/cm3_fault_handlers.c
    #endif

    ## Misc library functions
    SRC += $(FLIGHTLIB)/printf2.c
    SRC += $(FLIGHTLIB)/WorldMagModel.c

    ## UAVObjects
    SRC += $(OPUAVSYNTHDIR)/objectpersistence.c
    SRC += $(OPUAVSYNTHDIR)/gcstelemetrystats.c
    SRC += $(OPUAVSYNTHDIR)/flighttelemetrystats.c
    SRC += $(OPUAVSYNTHDIR)/flightstatus.c
    SRC += $(OPUAVSYNTHDIR)/systemstats.c
    SRC += $(OPUAVSYNTHDIR)/systemalarms.c
    SRC += $(OPUAVSYNTHDIR)/systemsettings.c
    SRC += $(OPUAVSYNTHDIR)/attitudeactual.c
    SRC += $(OPUAVSYNTHDIR)/i2cstats.c
    SRC += $(OPUAVSYNTHDIR)/watchdogstatus.c
    SRC += $(OPUAVSYNTHDIR)/firmwareiapobj.c
    SRC += $(OPUAVSYNTHDIR)/hwsettings.c
    SRC += $(OPUAVSYNTHDIR)/taskinfo.c
    SRC += $(OPUAVSYNTHDIR)/mixerstatus.c
    SRC += $(OPUAVSYNTHDIR)/homelocation.c
    SRC += $(OPUAVSYNTHDIR)/gpsposition.c
    SRC += $(OPUAVSYNTHDIR)/gpssatellites.c
    SRC += $(OPUAVSYNTHDIR)/gpsvelocity.c
    SRC += $(OPUAVSYNTHDIR)/gpstime.c
    SRC += $(OPUAVSYNTHDIR)/osdsettings.c
    SRC += $(OPUAVSYNTHDIR)/baroaltitude.c
    SRC += $(OPUAVSYNTHDIR)/magnetometer.c
else
    ## Test Code
    SRC += $(OPTESTS)/test_common.c
    SRC += $(OPTESTS)/$(TESTAPP).c
endif

	
# Optional component libraries
include $(PIOS)/Common/Libraries/dosfs/library.mk

include $(ROOT_DIR)/make/apps-defs.mk
include $(ROOT_DIR)/make/common-defs.mk
