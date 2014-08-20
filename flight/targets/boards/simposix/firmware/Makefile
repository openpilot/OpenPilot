#####
#
# TODO: This file should be reworked. It will be done as a part of sim target refactoring.
#
# The OpenPilot Team, http://www.openpilot.org, Copyright (C) 2009.
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

override ARM_SDK_PREFIX :=
override THUMB :=

include ../board-info.mk
include $(ROOT_DIR)/make/firmware-defs.mk

# Set developer code and compile options
# Set to YES for debugging
DEBUG ?= YES

# List of modules to include
MODULES = ManualControl Stabilization GPS
MODULES += PathPlanner
MODULES += PathFollower
MODULES += CameraStab
MODULES += Telemetry
MODULES += Logging
MODULES += FirmwareIAP
MODULES += StateEstimation
#MODULES += Sensors/simulated/Sensors
MODULES += Airspeed
#MODULES += AltitudeHold # now integrated in Stabilization
#MODULES += OveroSync

SRC += $(FLIGHTLIB)/notification.c

# Paths
OPSYSTEM = .
BOARDINC = ..
OPSYSTEMINC = $(OPSYSTEM)/inc
OPUAVTALK = ../../../../uavtalk
OPUAVTALKINC = $(OPUAVTALK)/inc
OPUAVOBJ = ../../../../uavobjects
OPUAVOBJINC = $(OPUAVOBJ)/inc
PIOSINC = $(PIOS)/inc
OPMODULEDIR = ../../../../modules
FLIGHTLIB = ../../../../libraries
FLIGHTLIBINC = $(FLIGHTLIB)/inc
MATHLIB = $(FLIGHTLIB)/math
MATHLIBINC = $(FLIGHTLIB)/math
PIOSPOSIX = $(PIOS)/posix
PIOSCOMMON = $(PIOS)/posix
PIOSCORECOMMON = $(PIOS)/common
PIOSCOMMONLIB = $(PIOS)/common/libraries
BOOT = 
BOOTINC = 
UAVOBJSYNTHDIR = $(OUTDIR)/../uavobject-synthetics/flight

# optional component libraries
include $(PIOS)/common/libraries/FreeRTOS/library.mk
#include $(FLIGHTLIB)/PyMite/pymite.mk

# List C source files here. (C dependencies are automatically generated.)
# use file-extension c for "c-only"-files

## MODULES
SRC += ${foreach MOD, ${MODULES}, ${wildcard ${OPMODULEDIR}/${MOD}/*.c}}
SRC += ${OUTDIR}/InitMods.c
## OPENPILOT CORE:
SRC += ${OPMODULEDIR}/System/systemmod.c
SRC += $(OPSYSTEM)/simposix.c
SRC += $(OPSYSTEM)/pios_board.c
SRC += $(FLIGHTLIB)/alarms.c
SRC += $(OPUAVTALK)/uavtalk.c
SRC += $(OPUAVOBJ)/uavobjectmanager.c
SRC += $(OPUAVOBJ)/eventdispatcher.c
SRC += $(UAVOBJSYNTHDIR)/uavobjectsinit.c

SRC += $(FLIGHTLIB)/CoordinateConversions.c
SRC += $(FLIGHTLIB)/fifo_buffer.c
SRC += $(FLIGHTLIB)/WorldMagModel.c
SRC += $(FLIGHTLIB)/insgps13state.c
SRC += $(FLIGHTLIB)/paths.c
SRC += $(FLIGHTLIB)/plans.c
SRC += $(FLIGHTLIB)/sanitycheck.c

SRC += $(MATHLIB)/sin_lookup.c
SRC += $(MATHLIB)/pid.c
SRC += $(MATHLIB)/mathmisc.c
SRC += $(MATHLIB)/butterworth.c

SRC += $(PIOSCORECOMMON)/pios_task_monitor.c
SRC += $(PIOSCORECOMMON)/pios_dosfs_logfs.c
SRC += $(PIOSCORECOMMON)/pios_debuglog.c
SRC += $(PIOSCORECOMMON)/pios_callbackscheduler.c
SRC += $(PIOSCORECOMMON)/pios_deltatime.c
SRC += $(PIOSCORECOMMON)/pios_notify.c
SRC += $(PIOSCORECOMMON)/pios_mem.c

## PIOS Hardware
include $(PIOS)/posix/library.mk

include ./UAVObjects.inc
SRC += $(UAVOBJSRC)

# List any extra directories to look for include files here.
#    Each directory must be seperated by a space.
EXTRAINCDIRS  += $(PIOS)
EXTRAINCDIRS  += $(PIOSINC)
EXTRAINCDIRS  += $(BOARDINC)
EXTRAINCDIRS  += $(OPSYSTEMINC)
EXTRAINCDIRS  += $(OPUAVTALK)
EXTRAINCDIRS  += $(OPUAVTALKINC)
EXTRAINCDIRS  += $(OPUAVOBJ)
EXTRAINCDIRS  += $(OPUAVOBJINC)
EXTRAINCDIRS  += $(UAVOBJSYNTHDIR)
EXTRAINCDIRS  += $(FLIGHTLIBINC)
EXTRAINCDIRS  += $(MATHLIBINC)
EXTRAINCDIRS  += $(PIOSCOMMON)
EXTRAINCDIRS  += $(CMSISDIR)
EXTRAINCDIRS  += $(OPUAVSYNTHDIR)
EXTRAINCDIRS  += $(BOOTINC)

EXTRAINCDIRS += ${foreach MOD, ${MODULES}, $(OPMODULEDIR)/${MOD}/inc} ${OPMODULEDIR}/System/inc

BLONLY_CDEFS += -DBOARD_TYPE=$(BOARD_TYPE)
BLONLY_CDEFS += -DBOARD_REVISION=$(BOARD_REVISION)
BLONLY_CDEFS += -DHW_TYPE=$(HW_TYPE)
BLONLY_CDEFS += -DBOOTLOADER_VERSION=$(BOOTLOADER_VERSION)
BLONLY_CDEFS += -DFW_BANK_BASE=$(FW_BANK_BASE)
BLONLY_CDEFS += -DFW_BANK_SIZE=$(FW_BANK_SIZE)
BLONLY_CDEFS += -DFW_DESC_SIZE=$(FW_DESC_SIZE)

# Since we are simulating all this firmware the code needs to know what the BL would
# normally contain
CFLAGS += $(BLONLY_CDEFS)

ifeq ($(DEBUG),YES)
CFLAGS += -O0
CFLAGS += -DGENERAL_COV
CFLAGS += -finstrument-functions -ffixed-r10
else
CFLAGS += -Os
endif
   
# common architecture-specific flags from the device-specific library makefile
CFLAGS += $(ARCHFLAGS)

CFLAGS += $(UAVOBJDEFINE)

CFLAGS += -DDIAG_STACK
CFLAGS += -DDIA_MIXERSTATUS
CFLAGS += -DDIAG_RATEDESIRED
CFLAGS += -DDIAG_I2C_WDG_STATS
CFLAGS += -DDIAG_TASKS
# Or all of above:
#CFLAGS += -DDIAG_ALL

# This is not the best place for these.  Really should abstract out
# to the board file or something
#CFLAGS += -DSTM32F4XX
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

#CFLAGS += -mcpu=$(MCU)
CFLAGS += $(CDEFS)
CFLAGS += $(patsubst %,-I%,$(EXTRAINCDIRS)) -I.

#CFLAGS += -mapcs-frame
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
ASFLAGS  = $(ARCHFLAGS) -I. -x assembler-with-cpp
ASFLAGS += $(ADEFS)
ASFLAGS += -Wa,-adhlns=$(addprefix $(OUTDIR)/, $(notdir $(addsuffix .lst, $(basename $<))))
ASFLAGS += $(patsubst %,-I%,$(EXTRAINCDIRS))

MATH_LIB = -lm

# Linker flags.
#  -Wl,...:     tell GCC to pass this to linker.
#    -Map:      create map file
#    --cref:    add cross reference to  map file
#LDFLAGS = -nostartfiles -Wl,-Map=$(OUTDIR)/$(TARGET).map,--cref,--gc-sections
LDFLAGS = -Wl,-Map=$(OUTDIR)/$(TARGET).map,--cref,--gc-sections
LDFLAGS += $(patsubst %,-L%,$(EXTRA_LIBDIRS))
LDFLAGS += -lc
LDFLAGS += $(patsubst %,-l%,$(EXTRA_LIBS))
LDFLAGS += $(MATH_LIB)
LDFLAGS += -lc -lgcc -lpthread -lrt

#Linker scripts
LDFLAGS += $(addprefix -T,$(LINKER_SCRIPTS_APP))


# Define programs and commands.
REMOVE  = $(RM) -f

# List of all source files.
ALLSRC     = $(ASRCARM) $(ASRC) $(SRCARM) $(SRC) $(CPPSRCARM) $(CPPSRC)
# List of all source files without directory and file-extension.
ALLSRCBASE = $(notdir $(basename $(ALLSRC)))

# Define all object files.
ALLOBJ     = $(addprefix $(OUTDIR)/, $(addsuffix .o, $(ALLSRCBASE) uavobjectsinit.o.firmware_info ))

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

$(eval $(call OPFW_TEMPLATE,$(OUTDIR)/uavobjectsinit.o,$(BOARD_TYPE),$(BOARD_REVISION)))

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

#create compile-time module auto-initialisation
MODNAMES = ${notdir $(subst /revolution,,$(MODULES))}

# Generate intermediate code
gencode: ${OUTDIR}/InitMods.c ${OUTDIR}/pmlib_img.c ${OUTDIR}/pmlib_nat.c ${OUTDIR}/pmlibusr_img.c ${OUTDIR}/pmlibusr_nat.c ${OUTDIR}/pmfeatures.h 

# Generate code for module initialization
${OUTDIR}/InitMods.c: Makefile
	$(V1) $(ECHO) $(MSG_MODINIT $(call toprel, $@))
	$(V1) $(ECHO) $(QUOTE)// Autogenerated file$(QUOTE) > ${OUTDIR}/InitMods.c
	$(V1) $(ECHO) $(QUOTE)${foreach MOD, ${MODNAMES}, extern unsigned int ${MOD}Initialize(void);}$(QUOTE)  >> ${OUTDIR}/InitMods.c
	$(V1) $(ECHO) $(QUOTE)${foreach MOD, ${MODNAMES}, extern unsigned int ${MOD}Start(void);}$(QUOTE)  >> ${OUTDIR}/InitMods.c
	$(V1) $(ECHO) $(QUOTE)void InitModules() {$(QUOTE) >> ${OUTDIR}/InitMods.c
	$(V1) $(ECHO) $(QUOTE)${foreach MOD, ${MODNAMES}, ${MOD}Initialize();}$(QUOTE)  >> ${OUTDIR}/InitMods.c
	$(V1) $(ECHO) $(QUOTE)}$(QUOTE) >> ${OUTDIR}/InitMods.c
	$(V1) $(ECHO) $(QUOTE)void StartModules() {$(QUOTE) >> ${OUTDIR}/InitMods.c
	$(V1) $(ECHO) $(QUOTE)${foreach MOD, ${MODNAMES}, ${MOD}Start();}$(QUOTE)  >> ${OUTDIR}/InitMods.c
	$(V1) $(ECHO) $(QUOTE)}$(QUOTE) >> ${OUTDIR}/InitMods.c

# Listing of phony targets.
.PHONY : all build clean clean_list install
