#
# Copyright (c) 2009-2013, The OpenPilot Team, http://www.openpilot.org
# Copyright (c) 2012, PhoenixPilot, http://github.com/PhoenixPilot
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

ifndef OPENPILOT_IS_COOL
    $(error Top level Makefile must be used to build this target)
endif

include ../board-info.mk
include $(ROOT_DIR)/make/firmware-defs.mk

# ARM DSP library
USE_DSP_LIB ?= NO

# List of mandatory modules to include
MODULES += Sensors
#MODULES += Attitude/revolution
MODULES += StateEstimation # use instead of Attitude
MODULES += Altitude/revolution
MODULES += Airspeed
#MODULES += AltitudeHold # now integrated in Stabilization
MODULES += Stabilization
MODULES += ManualControl
MODULES += Receiver
MODULES += Actuator
MODULES += GPS
MODULES += TxPID
MODULES += CameraStab
MODULES += Battery
MODULES += FirmwareIAP
MODULES += Radio
MODULES += PathPlanner
MODULES += PathFollower
MODULES += Osd/osdoutout
MODULES += Logging
MODULES += Telemetry
MODULES += Notify

OPTMODULES += ComUsbBridge

SRC += $(FLIGHTLIB)/notification.c

# Include all camera options
CDEFS += -DUSE_INPUT_LPF -DUSE_GIMBAL_LPF -DUSE_GIMBAL_FF

# Some diagnostics
CDEFS += -DDIAG_TASKS

# Misc options
CFLAGS += -ffast-math

# List C source files here (C dependencies are automatically generated).
# Use file-extension c for "c-only"-files
ifndef TESTAPP
    ## Application Core
    SRC += ../pios_usb_board_data.c
    SRC += $(OPMODULEDIR)/System/systemmod.c
    SRC += $(OPSYSTEM)/revolution.c
    SRC += $(OPSYSTEM)/pios_board.c
    SRC += $(FLIGHTLIB)/alarms.c
    SRC += $(FLIGHTLIB)/instrumentation.c
    SRC += $(OPUAVTALK)/uavtalk.c
    SRC += $(OPUAVOBJ)/uavobjectmanager.c
    SRC += $(OPUAVOBJ)/uavobjectpersistence.c
    SRC += $(OPUAVOBJ)/eventdispatcher.c

    #ifeq ($(DEBUG), YES)
    SRC += $(OPSYSTEM)/dcc_stdio.c
    SRC += $(OPSYSTEM)/cm3_fault_handlers.c
    #endif

    ## Misc library functions
    SRC += $(FLIGHTLIB)/paths.c
    SRC += $(FLIGHTLIB)/plans.c
    SRC += $(FLIGHTLIB)/WorldMagModel.c
    SRC += $(FLIGHTLIB)/insgps13state.c
    SRC += $(FLIGHTLIB)/auxmagsupport.c
    SRC += $(FLIGHTLIB)/lednotification.c    
    SRC += $(FLIGHTLIB)/sha1.c

    ## UAVObjects
    include ./UAVObjects.inc
    SRC += $(UAVOBJSRC)
else
    ## Test Code
    SRC += $(OPTESTS)/test_common.c
    SRC += $(OPTESTS)/$(TESTAPP).c
endif

# Optional component libraries
include $(FLIGHTLIB)/rscode/library.mk

#include $(FLIGHTLIB)/PyMite/pymite.mk


include $(ROOT_DIR)/make/apps-defs.mk
include $(ROOT_DIR)/make/common-defs.mk
