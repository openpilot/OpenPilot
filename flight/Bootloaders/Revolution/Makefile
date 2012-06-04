 #####
 # Project: OpenPilot INS_BOOTLOADER
 #    
 #  
 # Makefile for OpenPilot INS project
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

WHEREAMI := $(dir $(lastword $(MAKEFILE_LIST)))
TOP      := $(realpath $(WHEREAMI)/../../../)
include $(TOP)/make/firmware-defs.mk
include $(TOP)/make/boards/$(BOARD_NAME)/board-info.mk

# Target file name (without extension).
TARGET := bl_$(BOARD_NAME)

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

# Paths
REVO_BL = $(WHEREAMI)
REVO_BLINC = $(REVO_BL)/inc
PIOS = ../../PiOS
PIOSINC = $(PIOS)/inc
FLIGHTLIB = ../../Libraries
FLIGHTLIBINC = ../../Libraries/inc
PIOSSTM32F4XX = $(PIOS)/STM32F4xx
PIOSCOMMON = $(PIOS)/Common
PIOSBOARDS = $(PIOS)/Boards
PIOSCOMMONLIB = $(PIOSCOMMON)/Libraries
APPLIBDIR = $(PIOSSTM32F4XX)/Libraries
STMLIBDIR = $(APPLIBDIR)
STMSPDDIR = $(STMLIBDIR)/STM32F4xx_StdPeriph_Driver
STMSPDSRCDIR = $(STMSPDDIR)/src
STMSPDINCDIR = $(STMSPDDIR)/inc
HWDEFSINC = ../../board_hw_defs/$(BOARD_NAME)

# List C source files here. (C dependencies are automatically generated.)
# use file-extension c for "c-only"-files

## BOOTLOADER:
SRC += main.c
SRC += pios_board.c
SRC += pios_usb_board_data.c
SRC += op_dfu.c

## PIOS Hardware (STM32F4xx)
include $(PIOS)/STM32F4xx/library.mk

# PIOS Hardware (Common)
SRC += $(PIOSCOMMON)/pios_board_info.c
SRC += $(PIOSCOMMON)/pios_com_msg.c
SRC += $(PIOSCOMMON)/printf-stdarg.c
SRC += $(PIOSCOMMON)/pios_usb_desc_hid_only.c

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

# List any extra directories to look for include files here.
#    Each directory must be seperated by a space.
EXTRAINCDIRS  += $(PIOS)
EXTRAINCDIRS  += $(PIOSINC)
EXTRAINCDIRS  += $(FLIGHTLIBINC)
EXTRAINCDIRS  += $(PIOSSTM34FXX)
EXTRAINCDIRS  += $(PIOSCOMMON)
EXTRAINCDIRS  += $(PIOSBOARDS)
EXTRAINCDIRS  += $(STMSPDINCDIR)
EXTRAINCDIRS  += $(CMSISDIR)
EXTRAINCDIRS  += $(REVO_BLINC)
EXTRAINCDIRS  += $(HWDEFSINC)

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
LINKERSCRIPTPATH = $(PIOSSTM32FXX)

# Optimization level, can be [0, 1, 2, 3, s]. 
# 0 = turn off optimization. s = optimize for size.
# (Note: 3 is not always the best optimization level. See avr-libc FAQ.)

ifeq ($(DEBUG),YES)
OPT = 0
else
OPT = s
endif

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
CDEFS = -DSTM32F4XX
CDEFS += -DSYSCLK_FREQ=$(SYSCLK_FREQ)
CDEFS += -DHSE_VALUE=$(OSCILLATOR_FREQ)
CDEFS += -DUSE_STDPERIPH_DRIVER
CDEFS += -DUSE_$(BOARD)

# Provide (only) the bootloader with board-specific defines
BLONLY_CDEFS += -DBOARD_TYPE=$(BOARD_TYPE)
BLONLY_CDEFS += -DBOARD_REVISION=$(BOARD_REVISION)
BLONLY_CDEFS += -DHW_TYPE=$(HW_TYPE)
BLONLY_CDEFS += -DBOOTLOADER_VERSION=$(BOOTLOADER_VERSION)
BLONLY_CDEFS += -DFW_BANK_BASE=$(FW_BANK_BASE)
BLONLY_CDEFS += -DFW_BANK_SIZE=$(FW_BANK_SIZE)
BLONLY_CDEFS += -DFW_DESC_SIZE=$(FW_DESC_SIZE)

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

ifeq ($(DEBUG),YES)
CFLAGS = 
endif

# This is not the best place for these.  Really should abstract out
# to the board file or something
CFLAGS += -DSTM32F4XX
CFLAGS += -DMEM_SIZE=1024000000

CFLAGS += -g$(DEBUGF)
CFLAGS += -O$(OPT)
ifeq ($(DEBUG),NO)
CFLAGS += -fdata-sections -ffunction-sections
endif
CFLAGS += -mcpu=$(MCU)
CFLAGS += $(CDEFS)
CFLAGS += $(BLONLY_CDEFS)
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
ASFLAGS  = -mcpu=$(MCU) -I. -x assembler-with-cpp
ASFLAGS += $(ADEFS)
ASFLAGS += -Wa,-adhlns=$(addprefix $(OUTDIR)/, $(notdir $(addsuffix .lst, $(basename $<))))
ASFLAGS += $(patsubst %,-I%,$(EXTRAINCDIRS))

MATH_LIB = -lm

# Linker flags.
#  -Wl,...:     tell GCC to pass this to linker.
#    -Map:      create map file
#    --cref:    add cross reference to  map file
LDFLAGS = -nostartfiles -Wl,-Map=$(OUTDIR)/$(TARGET).map,--cref,--gc-sections
ifeq ($(DEBUG),NO)
LDFLAGS += -Wl,-static
endif
LDFLAGS += $(patsubst %,-L%,$(EXTRA_LIBDIRS))
LDFLAGS += -lc
LDFLAGS += $(patsubst %,-l%,$(EXTRA_LIBS))
LDFLAGS += $(MATH_LIB)
LDFLAGS += -lc -lgcc 

#Linker scripts                                                                                                                                                                                                              
LDFLAGS += $(addprefix -T,$(LINKER_SCRIPTS_BL))

# Define programs and commands.
REMOVE  = $(REMOVE_CMD) -f

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
all: build

ifeq ($(LOADFORMAT),ihex)
build: elf hex sym
else 
ifeq ($(LOADFORMAT),binary)
build: elf bin sym
else 
ifeq ($(LOADFORMAT),both)
build: elf hex bin sym
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
$(eval $(call JTAG_TEMPLATE,$(OUTDIR)/$(TARGET).bin,$(BL_BANK_BASE),$(BL_BANK_SIZE),$(OPENOCD_JTAG_CONFIG),$(OPENOCD_CONFIG)))

.PHONY: elf lss sym hex bin bino
elf: $(OUTDIR)/$(TARGET).elf
lss: $(OUTDIR)/$(TARGET).lss 
sym: $(OUTDIR)/$(TARGET).sym
hex: $(OUTDIR)/$(TARGET).hex
bin: $(OUTDIR)/$(TARGET).bin
bino: $(OUTDIR)/$(TARGET).bin.o

# Display sizes of sections.
$(eval $(call SIZE_TEMPLATE, $(OUTDIR)/$(TARGET).elf))

# Generate Doxygen documents
docs:
	doxygen  $(DOXYGENDIR)/doxygen.cfg

# Install: install binary file with prefix/suffix into install directory
install: $(OUTDIR)/$(TARGET).bin
ifneq ($(INSTALL_DIR),)
	@echo $(MSG_INSTALLING) $(call toprel, $<)
	$(V1) mkdir -p $(INSTALL_DIR)
	$(V1) $(INSTALL) $< $(INSTALL_DIR)/$(INSTALL_PFX)$(TARGET)$(INSTALL_SFX).bin
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
-include $(shell mkdir -p $(OUTDIR) 2>/dev/null) $(shell mkdir $(OUTDIR)/dep 2>/dev/null) $(wildcard $(OUTDIR)/dep/*)
endif

# Listing of phony targets.
.PHONY : all build clean clean_list install
