#####
# Project: OpenPilot RevoMini
#
#
# Makefile for OpenPilot RevoMini project
#
# The OpenPilot Team, http://www.openpilot.org, Copyright (C) 2012.
#
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

WHEREAMI := $(dir $(lastword $(MAKEFILE_LIST)))
TOP      := $(realpath $(WHEREAMI)/../../)
include $(TOP)/make/firmware-defs.mk
include $(TOP)/make/boards/$(BOARD_NAME)/board-info.mk

# Target file name (without extension).
TARGET := fw_$(BOARD_NAME)

# Directory for output files (lst, obj, dep, elf, sym, map, hex, bin etc.)
OUTDIR := $(TOP)/build/$(TARGET)

# Set developer code and compile options
# Set to YES for debugging
DEBUG ?= NO

# Set to YES when using Code Sourcery toolchain
CODE_SOURCERY ?= NO

ifeq ($(CODE_SOURCERY), YES)
REMOVE_CMD = cs-rm
else
REMOVE_CMD = rm
endif

FLASH_TOOL = OPENOCD

# List of modules to include
MODULES = Sensors
MODULES += Attitude/revolution 
MODULES += ManualControl Stabilization Actuator
MODULES += Battery
MODULES += Altitude/revolution
MODULES += GPS FirmwareIAP
#MODULES += Airspeed/revolution
MODULES += AltitudeHold VtolPathFollower FixedWingPathFollower PathPlanner TxPID
MODULES += CameraStab
MODULES += Radio
MODULES += Osd/OsdHk
MODULES += Telemetry
PYMODULES = 
#FlightPlan

# Paths
OPSYSTEM = ./System
OPSYSTEMINC = $(OPSYSTEM)/inc
OPUAVTALK = ../UAVTalk
OPUAVTALKINC = $(OPUAVTALK)/inc
OPUAVOBJ = ../UAVObjects
OPUAVOBJINC = $(OPUAVOBJ)/inc
PIOS = ../PiOS
PIOSINC = $(PIOS)/inc
OPMODULEDIR = ../Modules
FLIGHTLIB = ../Libraries
FLIGHTLIBINC = ../Libraries/inc
MATHLIB = ../Libraries/math
MATHLIBINC = ../Libraries/math
RSCODE = $(FLIGHTLIB)/rscode
RSCODEINC = $(FLIGHTLIB)/rscode
PIOSSTM32F4XX = $(PIOS)/STM32F4xx
PIOSCOMMON = $(PIOS)/Common
PIOSBOARDS = $(PIOS)/Boards
PIOSCOMMONLIB = $(PIOSCOMMON)/Libraries
APPLIBDIR = $(PIOSSTM32F4XX)/Libraries
STMLIBDIR = $(APPLIBDIR)
STMSPDDIR = $(STMLIBDIR)/STM32F4xx_StdPeriph_Driver
STMSPDSRCDIR = $(STMSPDDIR)/src
STMSPDINCDIR = $(STMSPDDIR)/inc
OPUAVOBJ = ../UAVObjects
OPUAVOBJINC = $(OPUAVOBJ)/inc
PYMITE = $(FLIGHTLIB)/PyMite
PYMITELIB = $(PYMITE)/lib
PYMITEPLAT = $(PYMITE)/platform/openpilot
PYMITETOOLS = $(PYMITE)/tools
PYMITEVM = $(PYMITE)/vm
PYMITEINC = $(PYMITEVM)
PYMITEINC += $(PYMITEPLAT)
PYMITEINC += $(OUTDIR)
FLIGHTPLANLIB = $(OPMODULEDIR)/FlightPlan/lib
FLIGHTPLANS = $(OPMODULEDIR)/FlightPlan/flightplans
HWDEFSINC = ../board_hw_defs/$(BOARD_NAME)
UAVOBJSYNTHDIR = $(OUTDIR)/../uavobject-synthetics/flight

SRC = 
# optional component libraries
include $(PIOSCOMMONLIB)/FreeRTOS/library.mk
#include $(PIOSCOMMONLIB)/dosfs/library.mk
include $(PIOSCOMMONLIB)/msheap/library.mk


# List C source files here. (C dependencies are automatically generated.)
# use file-extension c for "c-only"-files

## PyMite files and modules
SRC += $(OUTDIR)/pmlib_img.c
SRC += $(OUTDIR)/pmlib_nat.c
SRC += $(OUTDIR)/pmlibusr_img.c
SRC += $(OUTDIR)/pmlibusr_nat.c
PYSRC += $(wildcard ${PYMITEVM}/*.c)
PYSRC += $(wildcard ${PYMITEPLAT}/*.c)
PYSRC += ${foreach MOD, ${PYMODULES}, ${wildcard ${OPMODULEDIR}/${MOD}/*.c}}
SRC += $(PYSRC)

## MODULES
SRC += ${foreach MOD, ${MODULES}, ${wildcard ${OPMODULEDIR}/${MOD}/*.c}}
## OPENPILOT CORE:
SRC += ${OPMODULEDIR}/System/systemmod.c
SRC += $(OPSYSTEM)/revolution.c
SRC += $(OPSYSTEM)/pios_board.c
SRC += $(OPSYSTEM)/pios_usb_board_data.c
SRC += $(OPSYSTEM)/alarms.c
SRC += $(OPUAVTALK)/uavtalk.c
SRC += $(OPUAVOBJ)/uavobjectmanager.c
SRC += $(OPUAVOBJ)/eventdispatcher.c

#ifeq ($(DEBUG),YES)
SRC += $(OPSYSTEM)/dcc_stdio.c
SRC += $(OPSYSTEM)/cm3_fault_handlers.c
#endif

SRC += $(FLIGHTLIB)/CoordinateConversions.c
SRC += $(FLIGHTLIB)/paths.c
SRC += $(FLIGHTLIB)/fifo_buffer.c
SRC += $(FLIGHTLIB)/WorldMagModel.c
SRC += $(FLIGHTLIB)/insgps13state.c
SRC += $(FLIGHTLIB)/taskmonitor.c
SRC += $(MATHLIB)/sin_lookup.c
SRC += $(MATHLIB)/pid.c

## For RFM22b
SRC += $(FLIGHTLIB)/packet_handler.c
SRC += $(RSCODE)/berlekamp.c
SRC += $(RSCODE)/crcgen.c
SRC += $(RSCODE)/galois.c
SRC += $(RSCODE)/rs.c

## PIOS Hardware (STM32F4xx)
include $(PIOS)/STM32F4xx/library.mk

## PIOS Hardware (Common)
SRC += $(PIOSCOMMON)/pios_mpu6000.c
SRC += $(PIOSCOMMON)/pios_bma180.c
SRC += $(PIOSCOMMON)/pios_etasv3.c
SRC += $(PIOSCOMMON)/pios_gcsrcvr.c
SRC += $(PIOSCOMMON)/pios_l3gd20.c
SRC += $(PIOSCOMMON)/pios_hmc5883.c
SRC += $(PIOSCOMMON)/pios_ms5611.c
SRC += $(PIOSCOMMON)/pios_crc.c
SRC += $(PIOSCOMMON)/pios_com.c
SRC += $(PIOSCOMMON)/pios_rfm22b.c
SRC += $(PIOSCOMMON)/pios_rcvr.c
SRC += $(PIOSCOMMON)/pios_sbus.c
SRC += $(PIOSCOMMON)/pios_flash_jedec.c
SRC += $(PIOSCOMMON)/pios_flashfs_objlist.c
SRC += $(PIOSCOMMON)/printf-stdarg.c
SRC += $(PIOSCOMMON)/pios_usb_desc_hid_cdc.c
SRC += $(PIOSCOMMON)/pios_usb_desc_hid_only.c
SRC += $(PIOSCOMMON)/pios_usb_util.c

include ./UAVObjects.inc
SRC += $(UAVOBJSRC)

# List C source files here which must be compiled in ARM-Mode (no -mthumb).
# use file-extension c for "c-only"-files
## just for testing, timer.c could be compiled in thumb-mode too
SRCARM =

# List C++ source files here.
# use file-extension .cpp for C++-files (not .C)
CPPSRC =

# List C++ source files here which must be compiled in ARM-Mode.
# use file-extension .cpp for C++-files (not .C)
#CPPSRCARM = $(TARGET).cpp
CPPSRCARM =

# List Assembler source files here.
# Make them always end in a capital .S. Files ending in a lowercase .s
# will not be considered source files but generated files (assembler
# output from the compiler), and will be deleted upon "make clean"!
# Even though the DOS/Win* filesystem matches both .s and .S the same,
# it will preserve the spelling of the filenames, and gcc itself does
# care about how the name is spelled on its command-line.


# List Assembler source files here which must be assembled in ARM-Mode..
ASRCARM =

# List any extra directories to look for include files here.
#    Each directory must be seperated by a space.
EXTRAINCDIRS  += $(PIOS)
EXTRAINCDIRS  += $(PIOSINC)
EXTRAINCDIRS  += $(OPSYSTEMINC)
EXTRAINCDIRS  += $(OPUAVTALK)
EXTRAINCDIRS  += $(OPUAVTALKINC)
EXTRAINCDIRS  += $(OPUAVOBJ)
EXTRAINCDIRS  += $(OPUAVOBJINC)
EXTRAINCDIRS  += $(UAVOBJSYNTHDIR)
EXTRAINCDIRS  += $(FLIGHTLIBINC)
EXTRAINCDIRS  += $(MATHLIBINC)
EXTRAINCDIRS  += $(RSCODEINC)
EXTRAINCDIRS  += $(PIOSSTM32F4XX)
EXTRAINCDIRS  += $(PIOSCOMMON)
EXTRAINCDIRS  += $(PIOSBOARDS)
EXTRAINCDIRS  += $(STMSPDINCDIR)
EXTRAINCDIRS  += $(CMSISDIR)
EXTRAINCDIRS  += $(OPUAVSYNTHDIR)
EXTRAINCDIRS  += $(BOOTINC)
EXTRAINCDIRS  += $(PYMITEINC)
EXTRAINCDIRS  += $(HWDEFSINC)

# Generate intermediate code
gencode: ${OUTDIR}/pmlib_img.c ${OUTDIR}/pmlib_nat.c ${OUTDIR}/pmlibusr_img.c ${OUTDIR}/pmlibusr_nat.c ${OUTDIR}/pmfeatures.h 

$(PYSRC): gencode

PYTHON = python

# Generate code for PyMite
${OUTDIR}/pmlib_img.c ${OUTDIR}/pmlib_nat.c ${OUTDIR}/pmlibusr_img.c ${OUTDIR}/pmlibusr_nat.c ${OUTDIR}/pmfeatures.h: $(wildcard ${PYMITELIB}/*.py) $(wildcard ${PYMITEPLAT}/*.py) $(wildcard ${FLIGHTPLANLIB}/*.py) $(wildcard ${FLIGHTPLANS}/*.py) 
	@echo $(MSG_PYMITEINIT) $(call toprel, $@)
	@$(PYTHON) $(PYMITETOOLS)/pmImgCreator.py -f $(PYMITEPLAT)/pmfeatures.py -c -s --memspace=flash -o $(OUTDIR)/pmlib_img.c --native-file=$(OUTDIR)/pmlib_nat.c $(PYMITELIB)/list.py $(PYMITELIB)/dict.py $(PYMITELIB)/__bi.py $(PYMITELIB)/sys.py $(PYMITELIB)/string.py $(wildcard $(FLIGHTPLANLIB)/*.py)
	@$(PYTHON) $(PYMITETOOLS)/pmGenPmFeatures.py $(PYMITEPLAT)/pmfeatures.py > $(OUTDIR)/pmfeatures.h
	@$(PYTHON) $(PYMITETOOLS)/pmImgCreator.py -f $(PYMITEPLAT)/pmfeatures.py -c -u -o $(OUTDIR)/pmlibusr_img.c --native-file=$(OUTDIR)/pmlibusr_nat.c $(FLIGHTPLANS)/test.py
EXTRAINCDIRS += ${foreach MOD, ${MODULES} ${PYMODULES}, $(OPMODULEDIR)/${MOD}/inc} ${OPMODULEDIR}/System/inc

# List any extra directories to look for library files here.
# Also add directories where the linker should search for
# includes from linker-script to the list
#     Each directory must be seperated by a space.
EXTRA_LIBDIRS =

# Extra Libraries
#    Each library-name must be seperated by a space.
#    i.e. to link with libxyz.a, libabc.a and libefsl.a:
#    EXTRA_LIBS = xyz abc efsl
# for newlib-lpc (file: libnewlibc-lpc.a):
#    EXTRA_LIBS = newlib-lpc
EXTRA_LIBS =

# Path to Linker-Scripts
LINKERSCRIPTPATH = $(PIOSSTM32F4XX)

# Optimization level, can be [0, 1, 2, 3, s].
# 0 = turn off optimization. s = optimize for size.
# (Note: 3 is not always the best optimization level. See avr-libc FAQ.)

ifeq ($(DEBUG),YES)
CFLAGS += -O0
CFLAGS += -DGENERAL_COV
CFLAGS += -finstrument-functions -ffixed-r10
else
CFLAGS += -Os
endif


   
# common architecture-specific flags from the device-specific library makefile
CFLAGS += $(ARCHFLAGS)

CFLAGS += -DDIAGNOSTICS
CFLAGS += -DDIAG_TASKS

# This is not the best place for these.  Really should abstract out
# to the board file or something
CFLAGS += -DSTM32F4XX
CFLAGS += -DMEM_SIZE=1024000000

# Output format. (can be ihex or binary or both)
#  binary to create a load-image in raw-binary format i.e. for SAM-BA,
#  ihex to create a load-image in Intel hex format
#LOADFORMAT = ihex
#LOADFORMAT = binary
LOADFORMAT = both

# Debugging format.
DEBUGF = dwarf-2

# Place project-specific -D (define) and/or
# -U options for C here.
CDEFS += -DHSE_VALUE=$(OSCILLATOR_FREQ)
CDEFS += -DSYSCLK_FREQ=$(SYSCLK_FREQ)
CDEFS += -DUSE_STDPERIPH_DRIVER
CDEFS += -DUSE_$(BOARD)
ifeq ($(ENABLE_DEBUG_CONSOLE), YES)
CDEFS += -DPIOS_INCLUDE_DEBUG_CONSOLE
endif

# Place project-specific -D and/or -U options for
# Assembler with preprocessor here.
#ADEFS = -DUSE_IRQ_ASM_WRAPPER
ADEFS = -D__ASSEMBLY__

# Compiler flag to set the C Standard level.
# c89   - "ANSI" C
# gnu89 - c89 plus GCC extensions
# c99   - ISO C99 standard (not yet fully implemented)
# gnu99 - c99 plus GCC extensions
CSTANDARD = -std=gnu99

#-----

# Compiler flags.

#  -g*:          generate debugging information
#  -O*:          optimization level
#  -f...:        tuning, see GCC manual and avr-libc documentation
#  -Wall...:     warning level
#  -Wa,...:      tell GCC to pass this to the assembler.
#    -adhlns...: create assembler listing
#
# Flags for C and C++ (arm-elf-gcc/arm-elf-g++)

CFLAGS += -g$(DEBUGF)

CFLAGS += -ffast-math

CFLAGS += -mcpu=$(MCU)
CFLAGS += $(CDEFS)
CFLAGS += $(patsubst %,-I%,$(EXTRAINCDIRS)) -I.

CFLAGS += -mapcs-frame
CFLAGS += -fomit-frame-pointer
ifeq ($(CODE_SOURCERY), YES)
CFLAGS += -fpromote-loop-indices
endif

CFLAGS += -Wall
#CFLAGS += -Werror
CFLAGS += -Wa,-adhlns=$(addprefix $(OUTDIR)/, $(notdir $(addsuffix .lst, $(basename $<))))
# Compiler flags to generate dependency files:
CFLAGS += -MD -MP -MF $(OUTDIR)/dep/$(@F).d

# flags only for C
#CONLYFLAGS += -Wnested-externs
CONLYFLAGS += $(CSTANDARD)

# Assembler flags.
#  -Wa,...:    tell GCC to pass this to the assembler.
#  -ahlns:     create listing
ASFLAGS  = $(ARCHFLAGS) -mthumb -I. -x assembler-with-cpp
ASFLAGS += $(ADEFS)
ASFLAGS += -Wa,-adhlns=$(addprefix $(OUTDIR)/, $(notdir $(addsuffix .lst, $(basename $<))))
ASFLAGS += $(patsubst %,-I%,$(EXTRAINCDIRS))

MATH_LIB = -lm

# Linker flags.
#  -Wl,...:     tell GCC to pass this to linker.
#    -Map:      create map file
#    --cref:    add cross reference to  map file
LDFLAGS = -nostartfiles -Wl,-Map=$(OUTDIR)/$(TARGET).map,--cref,--gc-sections
LDFLAGS += $(patsubst %,-L%,$(EXTRA_LIBDIRS))
LDFLAGS += -lc
LDFLAGS += $(patsubst %,-l%,$(EXTRA_LIBS))
LDFLAGS += $(MATH_LIB)
LDFLAGS += -lc -lgcc

#Linker scripts                                                                                                                                                                                                              
LDFLAGS += $(addprefix -T,$(LINKER_SCRIPTS_APP))


# Define programs and commands.
REMOVE  = $(REMOVE_CMD) -f
PYHON   = python

# List of all source files.
ALLSRC     = $(ASRCARM) $(ASRC) $(SRCARM) $(SRC) $(CPPSRCARM) $(CPPSRC)
# List of all source files without directory and file-extension.
ALLSRCBASE = $(notdir $(basename $(ALLSRC)))

# Define all object files.
ALLOBJ     = $(addprefix $(OUTDIR)/, $(addsuffix .o, $(ALLSRCBASE)))

# Define all listing files (used for make clean).
LSTFILES   = $(addprefix $(OUTDIR)/, $(addsuffix .lst, $(ALLSRCBASE)))
# Define all depedency-files (used for make clean).
DEPFILES   = $(addprefix $(OUTDIR)/dep/, $(addsuffix .o.d, $(ALLSRCBASE)))

# Default target.
all: gccversion build

ifeq ($(LOADFORMAT),ihex)
build: elf hex lss sym
else
ifeq ($(LOADFORMAT),binary)
build: elf bin lss sym
else
ifeq ($(LOADFORMAT),both)
build: elf hex bin lss sym
else
$(error "$(MSG_FORMATERROR) $(FORMAT)")
endif
endif
endif


# Link: create ELF output file from object files.
$(eval $(call LINK_TEMPLATE, $(OUTDIR)/$(TARGET).elf, $(ALLOBJ)))

# Assemble: create object files from assembler source files.
$(foreach src, $(ASRC), $(eval $(call ASSEMBLE_TEMPLATE, $(src))))

# Assemble: create object files from assembler source files. ARM-only
$(foreach src, $(ASRCARM), $(eval $(call ASSEMBLE_ARM_TEMPLATE, $(src))))

# Compile: create object files from C source files.
$(foreach src, $(SRC), $(eval $(call COMPILE_C_TEMPLATE, $(src))))

# Compile: create object files from C source files. ARM-only
$(foreach src, $(SRCARM), $(eval $(call COMPILE_C_ARM_TEMPLATE, $(src))))

# Compile: create object files from C++ source files.
$(foreach src, $(CPPSRC), $(eval $(call COMPILE_CPP_TEMPLATE, $(src))))

# Compile: create object files from C++ source files. ARM-only
$(foreach src, $(CPPSRCARM), $(eval $(call COMPILE_CPP_ARM_TEMPLATE, $(src))))

# Compile: create assembler files from C source files. ARM/Thumb
$(eval $(call PARTIAL_COMPILE_TEMPLATE, SRC))

# Compile: create assembler files from C source files. ARM only
$(eval $(call PARTIAL_COMPILE_ARM_TEMPLATE, SRCARM))

$(OUTDIR)/$(TARGET).bin.o: $(OUTDIR)/$(TARGET).bin

$(eval $(call OPFW_TEMPLATE,$(OUTDIR)/$(TARGET).bin,$(BOARD_TYPE),$(BOARD_REVISION)))

# Add jtag targets (program and wipe)
$(eval $(call JTAG_TEMPLATE,$(OUTDIR)/$(TARGET).bin,$(FW_BANK_BASE),$(FW_BANK_SIZE),$(OPENOCD_JTAG_CONFIG),$(OPENOCD_CONFIG)))

.PHONY: elf lss sym hex bin bino opfw
elf: $(OUTDIR)/$(TARGET).elf
lss: $(OUTDIR)/$(TARGET).lss
sym: $(OUTDIR)/$(TARGET).sym
hex: $(OUTDIR)/$(TARGET).hex
bin: $(OUTDIR)/$(TARGET).bin
bino: $(OUTDIR)/$(TARGET).bin.o
opfw: $(OUTDIR)/$(TARGET).opfw

# Display sizes of sections.
$(eval $(call SIZE_TEMPLATE, $(OUTDIR)/$(TARGET).elf))

# Generate Doxygen documents
docs:
	doxygen  $(DOXYGENDIR)/doxygen.cfg

# Install: install binary file with prefix/suffix into install directory
install: $(OUTDIR)/$(TARGET).opfw
ifneq ($(INSTALL_DIR),)
	@echo $(MSG_INSTALLING) $(call toprel, $<)
	$(V1) mkdir -p $(INSTALL_DIR)
	$(V1) $(INSTALL) $< $(INSTALL_DIR)/$(INSTALL_PFX)$(TARGET)$(INSTALL_SFX).opfw
else
	$(error INSTALL_DIR must be specified for $@)
endif

# Target: clean project.
clean: clean_list

clean_list :
	@echo $(MSG_CLEANING)
	$(V1) $(REMOVE) $(OUTDIR)/$(TARGET).map
	$(V1) $(REMOVE) $(OUTDIR)/$(TARGET).elf
	$(V1) $(REMOVE) $(OUTDIR)/$(TARGET).hex
	$(V1) $(REMOVE) $(OUTDIR)/$(TARGET).bin
	$(V1) $(REMOVE) $(OUTDIR)/$(TARGET).sym
	$(V1) $(REMOVE) $(OUTDIR)/$(TARGET).lss
	$(V1) $(REMOVE) $(OUTDIR)/$(TARGET).bin.o
	$(V1) $(REMOVE) $(ALLOBJ)
	$(V1) $(REMOVE) $(LSTFILES)
	$(V1) $(REMOVE) $(DEPFILES)
	$(V1) $(REMOVE) $(SRC:.c=.s)
	$(V1) $(REMOVE) $(SRCARM:.c=.s)
	$(V1) $(REMOVE) $(CPPSRC:.cpp=.s)
	$(V1) $(REMOVE) $(CPPSRCARM:.cpp=.s)

# Create output files directory
# all known MS Windows OS define the ComSpec environment variable
ifdef ComSpec
$(shell md $(subst /,\\,$(OUTDIR)) 2>NUL)
else
$(shell mkdir -p $(OUTDIR) 2>/dev/null)
endif

# Include the dependency files.
ifdef ComSpec
-include $(shell md $(subst /,\\,$(OUTDIR))\dep 2>NUL) $(wildcard $(OUTDIR)/dep/*)
else
-include $(shell mkdir $(OUTDIR) 2>/dev/null) $(shell mkdir $(OUTDIR)/dep 2>/dev/null) $(wildcard $(OUTDIR)/dep/*)
endif

# Listing of phony targets.
.PHONY : all build clean clean_list install
