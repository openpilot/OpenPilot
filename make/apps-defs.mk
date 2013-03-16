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

# Set developer code and compile options.
# Set to YES to compile for debugging
DEBUG ?= NO

# Set to YES to use the Servo output pins for debugging via scope or logic analyser
ENABLE_DEBUG_PINS ?= NO

# Include objects that are just nice information to show
STACK_DIAGNOSTICS		?= NO
MIXERSTATUS_DIAGNOSTICS		?= NO
RATEDESIRED_DIAGNOSTICS		?= NO
I2C_WDG_STATS_DIAGNOSTICS	?= NO
DIAG_TASKS			?= NO

# Or just turn on all the above diagnostics. WARNING: this consumes massive amounts of memory.
ALL_DIGNOSTICS			?= NO

# Paths
TOPDIR		= .
OPSYSTEM	= $(TOPDIR)/System
OPSYSTEMINC	= $(OPSYSTEM)/inc
PIOSINC		= $(PIOS)/inc
PIOSCOMMON	= $(PIOS)/Common
PIOSBOARDS	= $(PIOS)/Boards
FLIGHTLIBINC	= $(FLIGHTLIB)/inc

## UAVTalk and UAVObject manager
OPUAVOBJINC	= $(OPUAVOBJ)/inc
OPUAVTALKINC	= $(OPUAVTALK)/inc

## Math
MATHLIB		= $(FLIGHTLIB)/math
MATHLIBINC	= $(FLIGHTLIB)/math

## FreeRTOS support
FREERTOS_DIR	 = $(PIOSCOMMON)/Libraries/FreeRTOS
FREERTOS_SRC_DIR = $(FREERTOS_DIR)/Source
FREERTOS_INC_DIR = $(FREERTOS_SRC_DIR)/include

## Misc
DOXYGENDIR	= $(ROOT_DIR)/flight/Doc/Doxygen
OPTESTS		= $(TOPDIR)/Tests

## PIOS Hardware
ifeq ($(MCU),cortex-m3)
    include $(PIOS)/STM32F10x/library.mk
else ifeq ($(MCU),cortex-m4)
    include $(PIOS)/STM32F4xx/library.mk
else
    $(error Unsupported MCU: $(MCU))
endif

# List C source files here (C dependencies are automatically generated).
# Use file-extension c for "c-only"-files

## PIOS Hardware (Common)
SRC += $(PIOSCOMMON)/pios_crc.c
SRC += $(PIOSCOMMON)/pios_flashfs_logfs.c
SRC += $(PIOSCOMMON)/pios_flash_jedec.c
SRC += $(PIOSCOMMON)/pios_adxl345.c
SRC += $(PIOSCOMMON)/pios_mpu6000.c
SRC += $(PIOSCOMMON)/pios_com.c
SRC += $(PIOSCOMMON)/pios_sbus.c
SRC += $(PIOSCOMMON)/pios_rcvr.c
SRC += $(PIOSCOMMON)/pios_gcsrcvr.c
SRC += $(PIOSCOMMON)/pios_rfm22b.c
SRC += $(PIOSCOMMON)/pios_rfm22b_com.c
SRC += $(PIOSCOMMON)/printf-stdarg.c
SRC += $(PIOSCOMMON)/pios_i2c_esc.c
SRC += $(PIOSSTM32F10X)/pios_ppm_out.c

# PIOS USB related files
SRC += $(OPSYSTEM)/pios_usb_board_data.c
SRC += $(PIOSCOMMON)/pios_usb_desc_hid_cdc.c
SRC += $(PIOSCOMMON)/pios_usb_desc_hid_only.c
SRC += $(PIOSCOMMON)/pios_usb_util.c

## Libraries for flight calculations
SRC += $(FLIGHTLIB)/fifo_buffer.c
SRC += $(FLIGHTLIB)/taskmonitor.c
SRC += $(FLIGHTLIB)/packet_handler.c
SRC += $(FLIGHTLIB)/sanitycheck.c
SRC += $(FLIGHTLIB)/CoordinateConversions.c
SRC += $(MATHLIB)/sin_lookup.c
SRC += $(MATHLIB)/pid.c

## Common FreeRTOS files
SRC += $(FREERTOS_SRC_DIR)/list.c
SRC += $(FREERTOS_SRC_DIR)/queue.c
SRC += $(FREERTOS_SRC_DIR)/tasks.c

# List C source files here which must be compiled in ARM-Mode (no -mthumb).
# Use file-extension c for "c-only"-files
SRCARM +=

# List C++ source files here.
# Use file-extension .cpp for C++-files (not .C)
CPPSRC +=

# List C++ source files here which must be compiled in ARM-Mode.
# Use file-extension .cpp for C++-files (not .C)
CPPSRCARM +=

# List Assembler source files here.
# Make them always end in a capital .S. Files ending in a lowercase .s
# will not be considered source files but generated files (assembler
# output from the compiler), and will be deleted upon "make clean"!
# Even though the DOS/Win* filesystem matches both .s and .S the same,
# it will preserve the spelling of the filenames, and gcc itself does
# care about how the name is spelled on its command-line.
ASRC +=

# List Assembler source files here which must be assembled in ARM-Mode.
ASRCARM +=

# List any extra directories to look for include files here.
#    Each directory must be seperated by a space.
EXTRAINCDIRS += $(PIOS)
EXTRAINCDIRS += $(PIOSINC)
EXTRAINCDIRS += $(FLIGHTLIBINC)
EXTRAINCDIRS += $(PIOSCOMMON)
EXTRAINCDIRS += $(PIOSBOARDS)
EXTRAINCDIRS += $(HWDEFSINC)
EXTRAINCDIRS += $(OPSYSTEMINC)
EXTRAINCDIRS += $(MATHLIBINC)
EXTRAINCDIRS += $(OPUAVOBJINC)
EXTRAINCDIRS += $(OPUAVTALKINC)
EXTRAINCDIRS += $(OPUAVSYNTHDIR)
EXTRAINCDIRS += $(FREERTOS_INC_DIR)

# Modules
EXTRAINCDIRS += $(foreach mod, $(OPTMODULES) $(MODULES), $(OPMODULEDIR)/$(mod)/inc) $(OPMODULEDIR)/System/inc

# List any extra directories to look for library files here.
# Also add directories where the linker should search for
# includes from linker-script to the list
#     Each directory must be seperated by a space.
EXTRA_LIBDIRS +=

# Extra Libraries
#    Each library-name must be seperated by a space.
#    i.e. to link with libxyz.a, libabc.a and libefsl.a:
#    EXTRA_LIBS = xyz abc efsl
# for newlib-lpc (file: libnewlibc-lpc.a):
#    EXTRA_LIBS = newlib-lpc
EXTRA_LIBS += m

# Compiler flags
CFLAGS +=

# Declare all non-optional modules as built-in to force inclusion
CDEFS += $(foreach mod, $(notdir $(MODULES)), -DMODULE_$(mod)_BUILTIN)

# The following Makefile command, ifneq (,$(filter) $(A), $(B) $(C))
#    is equivalent to the pseudocode `if (A == B || A == C)`
ifneq (,$(filter YES,$(STACK_DIAGNOSTICS) $(ALL_DIGNOSTICS)))
    CFLAGS += -DSTACK_DIAGNOSTICS
endif

ifneq (,$(filter YES,$(MIXERSTATUS_DIAGNOSTICS) $(ALL_DIGNOSTICS)))
    CFLAGS += -DMIXERSTATUS_DIAGNOSTICS
endif

ifneq (,$(filter YES,$(RATEDESIRED_DIAGNOSTICS) $(ALL_DIGNOSTICS)))
    CFLAGS += -DRATEDESIRED_DIAGNOSTICS
endif

ifneq (,$(filter YES,$(I2C_WDG_STATS_DIAGNOSTICS) $(ALL_DIGNOSTICS)))
    CFLAGS += -DI2C_WDG_STATS_DIAGNOSTICS
endif

ifneq (,$(filter YES,$(DIAG_TASKS) $(ALL_DIGNOSTICS)))
    CFLAGS += -DDIAG_TASKS
endif

# Set linker-script name depending on selected submodel name
ifeq ($(MCU),cortex-m3)
    LDFLAGS += -T$(LINKER_SCRIPTS_PATH)/link_$(BOARD)_memory.ld
    LDFLAGS += -T$(LINKER_SCRIPTS_PATH)/link_$(BOARD)_sections.ld
else ifeq ($(MCU),cortex-m4)
    LDFLAGS += $(addprefix -T,$(LINKER_SCRIPTS_APP))
endif
