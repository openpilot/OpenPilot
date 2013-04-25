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

include ../../boards/$(BOARD_NAME)/board-info.mk
include $(ROOT_DIR)/make/firmware-defs.mk

# ARM DSP library
override USE_DSP_LIB := NO

# Paths
TOPDIR		= .
OPSYSTEM	= $(TOPDIR)
OPSYSTEMINC	= $(OPSYSTEM)/inc
PIOSINC		= $(PIOS)/inc
PIOSCOMMON	= $(PIOS)/Common
FLIGHTLIBINC	= $(FLIGHTLIB)/inc
HWDEFSINC	= ../../boards/$(BOARD_NAME)

# List C source files here (C dependencies are automatically generated).
# Use file-extension c for "c-only"-files
SRC += $(OPSYSTEM)/main.c
SRC += $(OPSYSTEM)/pios_board.c

## PIOS Hardware
ifeq ($(MCU),cortex-m3)
    include $(PIOS)/STM32F10x/library.mk

    # Set linker-script name depending on selected submodel name
    LDFLAGS += -T$(LINKER_SCRIPTS_PATH)/link_$(BOARD)_memory.ld
    LDFLAGS += -T$(LINKER_SCRIPTS_PATH)/link_$(BOARD)_sections.ld
else
    $(error Unsupported MCU for BootloaderUpdater: $(MCU))
endif

# List any extra directories to look for include files here.
#    Each directory must be seperated by a space.
EXTRAINCDIRS += $(PIOS)
EXTRAINCDIRS += $(PIOSINC)
EXTRAINCDIRS += $(HWDEFSINC)
EXTRAINCDIRS += $(FLIGHTLIBINC)
EXTRAINCDIRS += $(PIOSCOMMON)
EXTRAINCDIRS += $(OPSYSTEMINC)

# Link: create ELF output file from object files.
EXTRAOBJ := $(BUILD_DIR)/bl_$(BOARD_NAME)/bl_$(BOARD_NAME).bin.o

include $(ROOT_DIR)/make/common-defs.mk
