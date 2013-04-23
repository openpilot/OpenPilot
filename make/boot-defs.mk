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

# Paths
TOPDIR		= .
OPSYSTEM	= $(TOPDIR)
BOARDINC	= $(TOPDIR)/..
OPSYSTEMINC	= $(OPSYSTEM)/inc
PIOSINC		= $(PIOS)/inc
PIOSCOMMON	= $(PIOS)/Common
FLIGHTLIBINC	= $(FLIGHTLIB)/inc

# ARM DSP library
override USE_DSP_LIB := NO

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

## Bootloader Core
SRC += ../pios_usb_board_data.c
SRC += $(OPSYSTEM)/main.c
SRC += $(OPSYSTEM)/pios_board.c

## PIOS Hardware (Common)
SRC += $(PIOSCOMMON)/pios_board_info.c
SRC += $(PIOSCOMMON)/pios_com_msg.c
SRC += $(PIOSCOMMON)/pios_iap.c
SRC += $(PIOSCOMMON)/pios_usb_desc_hid_only.c
SRC += $(PIOSCOMMON)/pios_usb_util.c

## Misc library functions
SRC += $(FLIGHTLIB)/op_dfu.c
SRC += $(FLIGHTLIB)/printf-stdarg.c

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
EXTRAINCDIRS += $(BOARDINC)
EXTRAINCDIRS += $(FLIGHTLIBINC)
EXTRAINCDIRS += $(PIOSCOMMON)
EXTRAINCDIRS += $(OPSYSTEMINC)

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
EXTRA_LIBS +=

# Provide (only) the bootloader with board-specific defines
BLONLY_CDEFS += -DBOARD_TYPE=$(BOARD_TYPE)
BLONLY_CDEFS += -DBOARD_REVISION=$(BOARD_REVISION)
BLONLY_CDEFS += -DHW_TYPE=$(HW_TYPE)
BLONLY_CDEFS += -DBOOTLOADER_VERSION=$(BOOTLOADER_VERSION)
BLONLY_CDEFS += -DFW_BANK_BASE=$(FW_BANK_BASE)
BLONLY_CDEFS += -DFW_BANK_SIZE=$(FW_BANK_SIZE)
BLONLY_CDEFS += -DFW_DESC_SIZE=$(FW_DESC_SIZE)

# Compiler flags
CDEFS += $(BLONLY_CDEFS)

# Set linker-script name depending on selected submodel name
ifeq ($(MCU),cortex-m3)
    LDFLAGS += -T$(LINKER_SCRIPTS_PATH)/link_$(BOARD)_memory.ld
    LDFLAGS += -T$(LINKER_SCRIPTS_PATH)/link_$(BOARD)_BL_sections.ld
else ifeq ($(MCU),cortex-m4)
    LDFLAGS += $(addprefix -T,$(LINKER_SCRIPTS_BL))
endif

# Add jtag targets (program and wipe)
$(eval $(call JTAG_TEMPLATE,$(OUTDIR)/$(TARGET).bin,$(BL_BANK_BASE),$(BL_BANK_SIZE),$(OPENOCD_JTAG_CONFIG),$(OPENOCD_CONFIG)))
