 #####
 # Project: OpenPilot Pip Modems
 #    
 #  
 # Makefile for OpenPilot Pip Modem project
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
TOP      := $(realpath $(WHEREAMI)/../../)
include $(TOP)/make/firmware-defs.mk

# Debugging (YES/NO) ?
DEBUG ?= NO

# Use Code Sourcery toolchain (YES/NO) ?
CODE_SOURCERY ?= YES

ifeq ($(CODE_SOURCERY), YES)
REMOVE_CMD = cs-rm
else
REMOVE_CMD = rm
endif

FLASH_TOOL = OPENOCD

# Include the USB files (YES/NO) ?
USE_USB = YES

# MCU name, submodel and board
# - MCU used for compiler-option (-mcpu)
# - MODEL used for linker-script name (-T) and passed as define
# - BOARD just passed as define (optional)
MCU      = cortex-m3
CHIP     = STM32F103CBT
BOARD    = STM32103CB_PIPXTREME
#CHIP     = STM32F103C8T
#BOARD    = STM32103C8_PIPXTREME
MODEL    = MD

# Directory for output files (lst, obj, dep, elf, sym, map, hex, bin etc.)
OUTDIR = $(TOP)/build/pipxtreme

# Target file name (without extension).
TARGET = PipXtreme

# Paths
HOME_DIR = ./
HOME_DIR_INC = $(HOME_DIR)/inc
PIOS = ../PiOS
PIOSINC = $(PIOS)/inc
FLIGHTLIB = ../Libraries
FLIGHTLIBINC = $(FLIGHTLIB)/inc
PIOSSTM32F10X = $(PIOS)/STM32F10x
PIOSCOMMON = $(PIOS)/Common
PIOSBOARDS = $(PIOS)/Boards
APPLIBDIR = $(PIOSSTM32F10X)/Libraries
STMLIBDIR = $(APPLIBDIR)
STMSPDDIR = $(STMLIBDIR)/STM32F10x_StdPeriph_Driver
ifeq ($(USE_USB), YES)
	STMUSBDIR = $(STMLIBDIR)/STM32_USB-FS-Device_Driver
endif
STMSPDSRCDIR = $(STMSPDDIR)/src
STMSPDINCDIR = $(STMSPDDIR)/inc
ifeq ($(USE_USB), YES)
	STMUSBSRCDIR = $(STMUSBDIR)/src
	STMUSBINCDIR = $(STMUSBDIR)/inc
endif
CMSISDIR  = $(STMLIBDIR)/CMSIS/Core/CM3
BOOT = ../Bootloaders/AHRS
BOOTINC = $(BOOT)/inc


# List C source files here. (C dependencies are automatically generated.)
# use file-extension c for "c-only"-files

## Core:
SRC += $(HOME_DIR)/main.c
SRC += $(HOME_DIR)/pios_board.c
SRC += $(HOME_DIR)/crc.c
SRC += $(HOME_DIR)/aes.c
SRC += $(HOME_DIR)/rfm22b.c
SRC += $(HOME_DIR)/packet_handler.c
SRC += $(HOME_DIR)/stream.c
SRC += $(HOME_DIR)/ppm.c
SRC += $(HOME_DIR)/transparent_comms.c
#SRC += $(HOME_DIR)/api_comms.c
SRC += $(HOME_DIR)/api_config.c
SRC += $(HOME_DIR)/saved_settings.c
SRC += $(HOME_DIR)/gpio_in.c
SRC += $(HOME_DIR)/stopwatch.c
SRC += $(HOME_DIR)/watchdog.c
SRC += $(FLIGHTLIB)/fifo_buffer.c
ifeq ($(USE_USB), YES)
	SRC += $(HOME_DIR)/pios_usb_hid_desc.c
endif


## PIOS Hardware (STM32F10x)
SRC += $(PIOSSTM32F10X)/pios_sys.c
SRC += $(PIOSSTM32F10X)/pios_led.c
SRC += $(PIOSSTM32F10X)/pios_delay.c
SRC += $(PIOSSTM32F10X)/pios_usart.c
SRC += $(PIOSSTM32F10X)/pios_irq.c
SRC += $(PIOSSTM32F10X)/pios_adc.c
SRC += $(PIOSSTM32F10X)/pios_gpio.c
SRC += $(PIOSSTM32F10X)/pios_spi.c
SRC += $(PIOSSTM32F10X)/pios_ppm.c

# PIOS USB related files (seperated to make code maintenance more easy)
ifeq ($(USE_USB), YES)
	SRC += $(PIOSSTM32F10X)/pios_usb_hid.c
	#SRC += $(PIOSSTM32F10X)/pios_usb_hid_desc.c
	#SRC += $(PIOSSTM32F10X)/pios_usb_hid_endp.c
	SRC += $(PIOSSTM32F10X)/pios_usb_hid_istr.c
	SRC += $(PIOSSTM32F10X)/pios_usb_hid_prop.c
	SRC += $(PIOSSTM32F10X)/pios_usb_hid_pwr.c
endif

## PIOS Hardware (Common)
SRC += $(PIOSCOMMON)/pios_com.c
SRC += $(PIOSCOMMON)/printf-stdarg.c

## CMSIS for STM32
SRC += $(CMSISDIR)/core_cm3.c
SRC += $(CMSISDIR)/system_stm32f10x.c

## Used parts of the STM-Library
SRC += $(STMSPDSRCDIR)/stm32f10x_adc.c
SRC += $(STMSPDSRCDIR)/stm32f10x_bkp.c
SRC += $(STMSPDSRCDIR)/stm32f10x_crc.c
SRC += $(STMSPDSRCDIR)/stm32f10x_dac.c
SRC += $(STMSPDSRCDIR)/stm32f10x_dbgmcu.c
SRC += $(STMSPDSRCDIR)/stm32f10x_dma.c
SRC += $(STMSPDSRCDIR)/stm32f10x_exti.c
SRC += $(STMSPDSRCDIR)/stm32f10x_flash.c
SRC += $(STMSPDSRCDIR)/stm32f10x_gpio.c
SRC += $(STMSPDSRCDIR)/stm32f10x_iwdg.c
SRC += $(STMSPDSRCDIR)/stm32f10x_i2c.c
SRC += $(STMSPDSRCDIR)/stm32f10x_pwr.c
SRC += $(STMSPDSRCDIR)/stm32f10x_rcc.c
SRC += $(STMSPDSRCDIR)/stm32f10x_rtc.c
SRC += $(STMSPDSRCDIR)/stm32f10x_spi.c
SRC += $(STMSPDSRCDIR)/stm32f10x_tim.c
SRC += $(STMSPDSRCDIR)/stm32f10x_usart.c
SRC += $(STMSPDSRCDIR)/misc.c

## STM32 USB Library
ifeq ($(USE_USB), YES)
	SRC += $(STMUSBSRCDIR)/usb_core.c
	SRC += $(STMUSBSRCDIR)/usb_init.c
	SRC += $(STMUSBSRCDIR)/usb_int.c
	SRC += $(STMUSBSRCDIR)/usb_mem.c
	SRC += $(STMUSBSRCDIR)/usb_regs.c
	SRC += $(STMUSBSRCDIR)/usb_sil.c
endif

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
ASRC = $(PIOSSTM32F10X)/startup_stm32f10x_$(MODEL).S

# List Assembler source files here which must be assembled in ARM-Mode..
ASRCARM = 

# List any extra directories to look for include files here.
#    Each directory must be seperated by a space.
EXTRAINCDIRS  += $(HOME_DIR)
EXTRAINCDIRS  += $(HOME_DIR_INC)
EXTRAINCDIRS  += $(PIOS)
EXTRAINCDIRS  += $(PIOSINC)
EXTRAINCDIRS  += $(FLIGHTLIBINC)
EXTRAINCDIRS  += $(PIOSSTM32F10X)
EXTRAINCDIRS  += $(PIOSCOMMON)
EXTRAINCDIRS  += $(PIOSBOARDS)
EXTRAINCDIRS  += $(STMSPDINCDIR)
ifeq ($(USE_USB), YES)
	EXTRAINCDIRS  += $(STMUSBINCDIR)
endif
EXTRAINCDIRS  += $(CMSISDIR)
EXTRAINCDIRS  += $(BOOTINC)

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
LINKERSCRIPTPATH = $(PIOSSTM32F10X)

# Optimization level, can be [0, 1, 2, 3, s]. 
# 0 = turn off optimization. s = optimize for size.
# (Note: 3 is not always the best optimization level. See avr-libc FAQ.)

ifeq ($(DEBUG),YES)
OPT = 0
else
OPT = s
#OPT = 2
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
CDEFS = -DSTM32F10X_$(MODEL)
CDEFS += -DUSE_STDPERIPH_DRIVER
CDEFS += -DUSE_$(BOARD)
CDEFS += -DUSE_BOOTLOADER

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
CFLAGS = -g$(DEBUGF)
endif

CFLAGS += -O$(OPT)
ifeq ($(DEBUG),NO)
CFLAGS += -fdata-sections -ffunction-sections
endif
CFLAGS += -mcpu=$(MCU)
CFLAGS += $(CDEFS)
CFLAGS += $(patsubst %,-I%,$(EXTRAINCDIRS)) -I.

CFLAGS += -mapcs-frame 
CFLAGS += -fomit-frame-pointer
ifeq ($(CODE_SOURCERY), YES)
CFLAGS += -fpromote-loop-indices
endif

CFLAGS += -Wall
CFLAGS += -Werror
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
LDFLAGS += -Wl,-static -Wl,-s
endif
LDFLAGS += $(patsubst %,-L%,$(EXTRA_LIBDIRS))
LDFLAGS += -lc
LDFLAGS += $(patsubst %,-l%,$(EXTRA_LIBS))
LDFLAGS += $(MATH_LIB)
LDFLAGS += -lc -lgcc 

# Set linker-script name depending on selected submodel name
LDFLAGS += -T$(LINKERSCRIPTPATH)/link_$(BOARD)_memory.ld
LDFLAGS += -T$(LINKERSCRIPTPATH)/link_$(BOARD)_sections.ld

OOCD_LOADFILE+=$(OUTDIR)/$(TARGET).bin
# Program                                                                                                                                                                                                                            
OOCD_CL+=-c "flash write_image erase $(OOCD_LOADFILE) 0x08003000 bin"
# Verify
OOCD_CL+=-c "verify_image $(OOCD_LOADFILE) 0x08003000 bin"
# reset target
OOCD_CL+=-c "reset run"
# terminate OOCD after programming
OOCD_CL+=-c shutdown

# Define programs and commands.
REMOVE  = $(REMOVE_CMD) -f
###SHELL   = sh
###COPY    = cp

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

# Program the device.
ifeq ($(FLASH_TOOL),OPENOCD)
# Program the device with Dominic Rath's OPENOCD in "batch-mode", needs cfg and "reset-script".
program: $(OUTDIR)/$(TARGET).bin
	@echo ${quote}Programming with OPENOCD${quote}
	$(OOCD_EXE) $(OOCD_CL)
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

.PHONY: elf lss sym hex bin bino
elf: $(OUTDIR)/$(TARGET).elf
lss: $(OUTDIR)/$(TARGET).lss 
sym: $(OUTDIR)/$(TARGET).sym
hex: $(OUTDIR)/$(TARGET).hex
bin: $(OUTDIR)/$(TARGET).bin
bino: $(OUTDIR)/$(TARGET).bin.o

# Display sizes of sections.
$(eval $(call SIZE_TEMPLATE, $(OUTDIR)/$(TARGET).elf))
.PHONY: size
size: $(OUTDIR)/$(TARGET).elf_size

# Generate Doxygen documents
docs:
	doxygen  $(DOXYGENDIR)/doxygen.cfg

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
.PHONY : all build clean clean_list program
