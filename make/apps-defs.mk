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
MATHLIB		= $(FLIGHTLIB)/math
MATHLIBINC	= $(FLIGHTLIB)/math
STMSPDSRCDIR	= $(STMSPDDIR)/src
STMSPDINCDIR	= $(STMSPDDIR)/inc
OPUAVTALKINC	= $(OPUAVTALK)/inc
OPUAVOBJINC	= $(OPUAVOBJ)/inc
DOXYGENDIR	= $(TOPDIR)/../Doc/Doxygen
OPTESTS		= $(TOPDIR)/Tests

PYMITE		= $(FLIGHTLIB)/PyMite
PYMITELIB	= $(PYMITE)/lib
PYMITEPLAT	= $(PYMITE)/platform/openpilot
PYMITETOOLS	= $(PYMITE)/tools
PYMITEVM	= $(PYMITE)/vm
PYMITEINC	= $(PYMITEVM)
PYMITEINC	+= $(PYMITEPLAT)
PYMITEINC	+= $(OUTDIR)

FLIGHTPLANLIB	= $(OPMODULEDIR)/FlightPlan/lib
FLIGHTPLANS	= $(OPMODULEDIR)/FlightPlan/flightplans

ifeq ($(MCU),cortex-m3)
    PIOSSTM32F10X = $(PIOS)/STM32F10x
    APPLIBDIR     = $(PIOSSTM32F10X)/Libraries
    STMLIBDIR     = $(APPLIBDIR)
    STMSPDDIR     = $(STMLIBDIR)/STM32F10x_StdPeriph_Driver
    STMUSBDIR     = $(STMLIBDIR)/STM32_USB-FS-Device_Driver
    STMUSBSRCDIR  = $(STMUSBDIR)/src
    STMUSBINCDIR  = $(STMUSBDIR)/inc
    CMSISDIR      = $(STMLIBDIR)/CMSIS/Core/CM3
    DOSFSDIR      = $(APPLIBDIR)/dosfs
    MSDDIR        = $(APPLIBDIR)/msd
    RTOSDIR       = $(PIOSCOMMON)/Libraries/FreeRTOS
    RTOSSRCDIR    = $(RTOSDIR)/Source
    RTOSINCDIR    = $(RTOSSRCDIR)/include
    RTOSPORTDIR   = $(APPLIBDIR)/FreeRTOS/Source
else ifeq ($(MCU),cortex-m4)
    $(error This makefile is not yet converted for F4, work in progress)
else
    $(error Unsupported MCU: $(MCU))
endif

# List C source files here (C dependencies are automatically generated).
# Use file-extension c for "c-only"-files

## PIOS Hardware
ifeq ($(MCU),cortex-m3)
    ## PIOS Hardware (STM32F10x)
    SRC += $(PIOSSTM32F10X)/pios_sys.c
    SRC += $(PIOSSTM32F10X)/pios_led.c
    SRC += $(PIOSSTM32F10X)/pios_delay.c
    SRC += $(PIOSSTM32F10X)/pios_usart.c
    SRC += $(PIOSSTM32F10X)/pios_irq.c
    SRC += $(PIOSSTM32F10X)/pios_adc.c
    SRC += $(PIOSSTM32F10X)/pios_servo.c
    SRC += $(PIOSSTM32F10X)/pios_i2c.c
    SRC += $(PIOSSTM32F10X)/pios_spi.c
    SRC += $(PIOSSTM32F10X)/pios_ppm.c
    SRC += $(PIOSSTM32F10X)/pios_pwm.c
    SRC += $(PIOSSTM32F10X)/pios_dsm.c
    SRC += $(PIOSSTM32F10X)/pios_debug.c
    SRC += $(PIOSSTM32F10X)/pios_gpio.c
    SRC += $(PIOSSTM32F10X)/pios_exti.c
    SRC += $(PIOSSTM32F10X)/pios_rtc.c
    SRC += $(PIOSSTM32F10X)/pios_wdg.c
    SRC += $(PIOSSTM32F10X)/pios_iap.c
    SRC += $(PIOSSTM32F10X)/pios_tim.c
    SRC += $(PIOSSTM32F10X)/pios_bl_helper.c
    SRC += $(PIOSSTM32F10X)/pios_eeprom.c
    SRC += $(PIOSSTM32F10X)/pios_ppm_out.c

    # PIOS USB related files
    SRC += $(OPSYSTEM)/pios_usb_board_data.c
    SRC += $(PIOSSTM32F10X)/pios_usb.c
    SRC += $(PIOSSTM32F10X)/pios_usbhook.c
    SRC += $(PIOSSTM32F10X)/pios_usb_hid.c
    SRC += $(PIOSSTM32F10X)/pios_usb_rctx.c
    SRC += $(PIOSSTM32F10X)/pios_usb_cdc.c
    SRC += $(PIOSSTM32F10X)/pios_usb_hid_istr.c
    SRC += $(PIOSSTM32F10X)/pios_usb_hid_pwr.c
    SRC += $(PIOSCOMMON)/pios_usb_desc_hid_cdc.c
    SRC += $(PIOSCOMMON)/pios_usb_desc_hid_only.c
    SRC += $(PIOSCOMMON)/pios_usb_util.c

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

    ## Libraries for flight calculations
    SRC += $(FLIGHTLIB)/fifo_buffer.c
    SRC += $(FLIGHTLIB)/CoordinateConversions.c
    SRC += $(FLIGHTLIB)/taskmonitor.c
    SRC += $(FLIGHTLIB)/sanitycheck.c
    SRC += $(MATHLIB)/sin_lookup.c
    SRC += $(MATHLIB)/pid.c

    ## CMSIS for STM32
    SRC += $(CMSISDIR)/core_cm3.c
    SRC += $(CMSISDIR)/system_stm32f10x.c

    ## Used parts of the STM-Library
    SRC += $(STMSPDSRCDIR)/stm32f10x_adc.c
    SRC += $(STMSPDSRCDIR)/stm32f10x_bkp.c
    SRC += $(STMSPDSRCDIR)/stm32f10x_crc.c
    SRC += $(STMSPDSRCDIR)/stm32f10x_dac.c
    SRC += $(STMSPDSRCDIR)/stm32f10x_dma.c
    SRC += $(STMSPDSRCDIR)/stm32f10x_exti.c
    SRC += $(STMSPDSRCDIR)/stm32f10x_flash.c
    SRC += $(STMSPDSRCDIR)/stm32f10x_gpio.c
    SRC += $(STMSPDSRCDIR)/stm32f10x_i2c.c
    SRC += $(STMSPDSRCDIR)/stm32f10x_pwr.c
    SRC += $(STMSPDSRCDIR)/stm32f10x_rcc.c
    SRC += $(STMSPDSRCDIR)/stm32f10x_rtc.c
    SRC += $(STMSPDSRCDIR)/stm32f10x_spi.c
    SRC += $(STMSPDSRCDIR)/stm32f10x_tim.c
    SRC += $(STMSPDSRCDIR)/stm32f10x_usart.c
    SRC += $(STMSPDSRCDIR)/stm32f10x_iwdg.c
    SRC += $(STMSPDSRCDIR)/stm32f10x_dbgmcu.c
    SRC += $(STMSPDSRCDIR)/misc.c

    ## STM32 USB Library
    SRC += $(STMUSBSRCDIR)/usb_core.c
    SRC += $(STMUSBSRCDIR)/usb_init.c
    SRC += $(STMUSBSRCDIR)/usb_int.c
    SRC += $(STMUSBSRCDIR)/usb_mem.c
    SRC += $(STMUSBSRCDIR)/usb_regs.c
    SRC += $(STMUSBSRCDIR)/usb_sil.c

    ## RTOS
    SRC += $(RTOSSRCDIR)/list.c
    SRC += $(RTOSSRCDIR)/queue.c
    SRC += $(RTOSSRCDIR)/tasks.c

    ## RTOS Portable
    SRC += $(RTOSPORTDIR)/portable/GCC/ARM_CM3/port.c
    SRC += $(RTOSPORTDIR)/portable/MemMang/heap_1.c
else ifeq ($(MCU),cortex-m4)
    ## PIOS Hardware (STM32F4xx)
    include $(PIOS)/STM32F4xx/library.mk
endif

# List C source files here which must be compiled in ARM-Mode (no -mthumb).
# Use file-extension c for "c-only"-files
SRCARM =

# List C++ source files here.
# Use file-extension .cpp for C++-files (not .C)
CPPSRC =

# List C++ source files here which must be compiled in ARM-Mode.
# Use file-extension .cpp for C++-files (not .C)
CPPSRCARM =

# List Assembler source files here.
# Make them always end in a capital .S. Files ending in a lowercase .s
# will not be considered source files but generated files (assembler
# output from the compiler), and will be deleted upon "make clean"!
# Even though the DOS/Win* filesystem matches both .s and .S the same,
# it will preserve the spelling of the filenames, and gcc itself does
# care about how the name is spelled on its command-line.
ifeq ($(MCU),cortex-m3)
    ASRC = $(PIOSSTM32F10X)/startup_stm32f10x_$(MODEL)$(MODEL_SUFFIX).S
else
    ASRC =
endif

# List Assembler source files here which must be assembled in ARM-Mode.
ASRCARM =

# List any extra directories to look for include files here.
#    Each directory must be seperated by a space.
EXTRAINCDIRS += $(PIOS)
EXTRAINCDIRS += $(PIOSINC)
EXTRAINCDIRS += $(FLIGHTLIBINC)
EXTRAINCDIRS += $(PIOSCOMMON)
EXTRAINCDIRS += $(PIOSBOARDS)
EXTRAINCDIRS += $(STMSPDINCDIR)
EXTRAINCDIRS += $(CMSISDIR)
EXTRAINCDIRS += $(HWDEFSINC)
EXTRAINCDIRS += $(OPSYSTEMINC)
EXTRAINCDIRS += $(OPUAVSYNTHDIR)
EXTRAINCDIRS += $(MATHLIBINC)
EXTRAINCDIRS += $(PYMITEINC)

ifeq ($(MCU),cortex-m3)
    EXTRAINCDIRS += $(PIOSSTM32F10X)
    EXTRAINCDIRS += $(OPUAVTALK)
    EXTRAINCDIRS += $(OPUAVTALKINC)
    EXTRAINCDIRS += $(OPUAVOBJ)
    EXTRAINCDIRS += $(OPUAVOBJINC)
    EXTRAINCDIRS += $(DOSFSDIR)
    EXTRAINCDIRS += $(MSDDIR)
    EXTRAINCDIRS += $(RTOSINCDIR)
    EXTRAINCDIRS += $(STMUSBINCDIR)
    EXTRAINCDIRS += $(APPLIBDIR)
    EXTRAINCDIRS += $(RTOSPORTDIR)/portable/GCC/ARM_CM3
else ifeq ($(MCU),cortex-m4)
    EXTRAINCDIRS += $(PIOSSTM34FXX)
endif

# Modules
EXTRAINCDIRS += $(foreach mod, $(OPTMODULES) $(MODULES), $(OPMODULEDIR)/$(mod)/inc) $(OPMODULEDIR)/System/inc

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

# Path to linker scripts
ifeq ($(MCU),cortex-m3)
    LINKERSCRIPTPATH = $(PIOSSTM32F10X)
else ifeq ($(MCU),cortex-m4)
    LINKERSCRIPTPATH = $(PIOSSTM32FXX)
endif

# Optimization level, can be [0, 1, 2, 3, s].
# 0 = turn off optimization. s = optimize for size.
# Note: 3 is not always the best optimization level.
ifeq ($(DEBUG), YES)
    OPT = 1
else
    OPT = s
endif

# Output format (can be ihex or binary or both).
#  binary to create a load-image in raw-binary format i.e. for SAM-BA,
#  ihex to create a load-image in Intel hex format
#LOADFORMAT = ihex
#LOADFORMAT = binary
LOADFORMAT = both

# Debugging format.
DEBUGF = dwarf-2

# Place project-specific -D (define) and/or -U options for C here.
ifeq ($(MCU),cortex-m3)
    CDEFS =  -DSTM32F10X_$(MODEL)
else ifeq ($(MCU),cortex-m4)
    CDEFS += -DSTM32F4XX
    CDEFS += -DSYSCLK_FREQ=$(SYSCLK_FREQ)
    CDEFS += -DHSE_VALUE=$(OSCILLATOR_FREQ)
endif

CDEFS += -DUSE_$(BOARD)
CDEFS += -DUSE_STDPERIPH_DRIVER

ifeq ($(ENABLE_DEBUG_PINS), YES)
    CDEFS += -DPIOS_ENABLE_DEBUG_PINS
endif

# Declare all non-optional modules as built-in to force inclusion
CDEFS += $(foreach mod, $(notdir $(MODULES)), -DMODULE_$(mod)_BUILTIN)

# Place project-specific -D and/or -U options for Assembler with preprocessor here.
#ADEFS = -DUSE_IRQ_ASM_WRAPPER
ADEFS = -D__ASSEMBLY__

# Compiler flag to set the C Standard level.
# c89   - "ANSI" C
# gnu89 - c89 plus GCC extensions
# c99   - ISO C99 standard (not yet fully implemented)
# gnu99 - c99 plus GCC extensions
CSTANDARD = -std=gnu99

# Compiler flags.
#
#  -g*:          generate debugging information
#  -O*:          optimization level
#  -f...:        tuning, see GCC manual and avr-libc documentation
#  -Wall...:     warning level
#  -Wa,...:      tell GCC to pass this to the assembler.
#    -adhlns...: create assembler listing

# Flags for C and C++ (arm-elf-gcc/arm-elf-g++)
ifeq ($(MCU),cortex-m4)
    # This is not the best place for these.  Really should abstract out
    # to the board file or something
    CFLAGS += -DSTM32F4XX
    CFLAGS += -DMEM_SIZE=1024000000
endif

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

CFLAGS += -mcpu=$(MCU)
CFLAGS += $(CDEFS)
CFLAGS += $(patsubst %,-I%,$(EXTRAINCDIRS)) -I.
CFLAGS += -mapcs-frame
CFLAGS += -fomit-frame-pointer
CFLAGS += -O$(OPT)
CFLAGS += -g$(DEBUGF)

ifeq ($(DEBUG), YES)
    CFLAGS += -DDEBUG
else
    CFLAGS += -fdata-sections -ffunction-sections
endif

CFLAGS += -Wall
# FIXME: STM32F4xx library raises strict aliasing and const qualifier warnings
ifneq ($(MCU),cortex-m4)
    CFLAGS += -Werror
endif
CFLAGS += -Wa,-adhlns=$(addprefix $(OUTDIR)/, $(notdir $(addsuffix .lst, $(basename $<))))

# Compiler flags to generate dependency files
CFLAGS += -MD -MP -MF $(OUTDIR)/dep/$(@F).d

# Flags only for C
#CONLYFLAGS += -Wnested-externs
CONLYFLAGS += $(CSTANDARD)

# Assembler flags.
#  -Wa,...:    tell GCC to pass this to the assembler.
#  -ahlns:     create listing
ASFLAGS =  -mcpu=$(MCU) -I. -x assembler-with-cpp
ASFLAGS += $(ADEFS)
ASFLAGS += -Wa,-adhlns=$(addprefix $(OUTDIR)/, $(notdir $(addsuffix .lst, $(basename $<))))
ASFLAGS += $(patsubst %,-I%,$(EXTRAINCDIRS))

MATH_LIB = -lm

# Linker flags.
#  -Wl,...:     tell GCC to pass this to linker.
#    -Map:      create map file
#    --cref:    add cross reference to  map file
LDFLAGS += -nostartfiles -Wl,-Map=$(OUTDIR)/$(TARGET).map,--cref,--gc-sections
LDFLAGS += $(patsubst %,-L%,$(EXTRA_LIBDIRS))
LDFLAGS += $(patsubst %,-l%,$(EXTRA_LIBS))
LDFLAGS += -lc -lgcc $(MATH_LIB)

ifneq ($(DEBUG), YES)
    LDFLAGS += -Wl,-static
endif

# Set linker-script name depending on selected submodel name
ifeq ($(MCU),cortex-m3)
    LDFLAGS += -T$(LINKERSCRIPTPATH)/link_$(BOARD)_memory.ld
    LDFLAGS += -T$(LINKERSCRIPTPATH)/link_$(BOARD)_sections.ld
else ifeq ($(MCU),cortex-m4)
    LDFLAGS += $(addprefix -T,$(LINKER_SCRIPTS_APP))
endif

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
else ifeq ($(LOADFORMAT),binary)
build: elf bin sym
else ifeq ($(LOADFORMAT),both)
build: elf hex bin sym
else
    $(error "$(MSG_FORMATERROR) $(FORMAT)")
endif

# Generate code for PyMite
# $(OUTDIR)/pmlib_img.c $(OUTDIR)/pmlib_nat.c $(OUTDIR)/pmlibusr_img.c $(OUTDIR)/pmlibusr_nat.c $(OUTDIR)/pmfeatures.h: $(wildcard $(PYMITELIB)/*.py) $(wildcard $(PYMITEPLAT)/*.py) $(wildcard $(FLIGHTPLANLIB)/*.py) $(wildcard $(FLIGHTPLANS)/*.py)
#	@echo $(MSG_PYMITEINIT) $(call toprel, $@)
#	@$(PYTHON) $(PYMITETOOLS)/pmImgCreator.py -f $(PYMITEPLAT)/pmfeatures.py -c -s --memspace=flash -o $(OUTDIR)/pmlib_img.c --native-file=$(OUTDIR)/pmlib_nat.c $(PYMITELIB)/list.py $(PYMITELIB)/dict.py $(PYMITELIB)/__bi.py $(PYMITELIB)/sys.py $(PYMITELIB)/string.py $(wildcard $(FLIGHTPLANLIB)/*.py)
#	@$(PYTHON) $(PYMITETOOLS)/pmGenPmFeatures.py $(PYMITEPLAT)/pmfeatures.py > $(OUTDIR)/pmfeatures.h
#	@$(PYTHON) $(PYMITETOOLS)/pmImgCreator.py -f $(PYMITEPLAT)/pmfeatures.py -c -u -o $(OUTDIR)/pmlibusr_img.c --native-file=$(OUTDIR)/pmlibusr_nat.c $(FLIGHTPLANS)/test.py

# Link: create ELF output file from object files.
$(eval $(call LINK_TEMPLATE, $(OUTDIR)/$(TARGET).elf, $(ALLOBJ), $(ALLLIB)))

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

# Add opfw target
$(eval $(call OPFW_TEMPLATE,$(OUTDIR)/$(TARGET).bin,$(BOARD_TYPE),$(BOARD_REVISION)))

# Add jtag targets (program and wipe)
$(eval $(call JTAG_TEMPLATE,$(OUTDIR)/$(TARGET).bin,$(BL_BANK_BASE),$(BL_BANK_SIZE),$(OPENOCD_JTAG_CONFIG),$(OPENOCD_CONFIG)))

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
	@$(ECHO) $(MSG_INSTALLING) $(call toprel, $<)
	$(V1) $(MKDIR) -p $(INSTALL_DIR)
	$(V1) $(INSTALL) $< $(INSTALL_DIR)/$(INSTALL_PFX)$(TARGET)$(INSTALL_SFX).opfw
else
	$(error INSTALL_DIR must be specified for $@)
endif

# Target: clean project.
clean: clean_list

clean_list :
	@echo $(MSG_CLEANING)
	$(V1) $(RM) -f $(OUTDIR)/$(TARGET).map
	$(V1) $(RM) -f $(OUTDIR)/$(TARGET).elf
	$(V1) $(RM) -f $(OUTDIR)/$(TARGET).hex
	$(V1) $(RM) -f $(OUTDIR)/$(TARGET).bin
	$(V1) $(RM) -f $(OUTDIR)/$(TARGET).sym
	$(V1) $(RM) -f $(OUTDIR)/$(TARGET).lss
	$(V1) $(RM) -f $(OUTDIR)/$(TARGET).bin.o
	$(V1) $(RM) -f $(OUTDIR)/$(TARGET).opfw
	$(V1) $(RM) -f $(wildcard $(OUTDIR)/*.c)
	$(V1) $(RM) -f $(wildcard $(OUTDIR)/*.h)
	$(V1) $(RM) -f $(ALLOBJ)
	$(V1) $(RM) -f $(LSTFILES)
	$(V1) $(RM) -f $(DEPFILES)
	$(V1) $(RM) -f $(SRC:.c=.s)
	$(V1) $(RM) -f $(SRCARM:.c=.s)
	$(V1) $(RM) -f $(CPPSRC:.cpp=.s)
	$(V1) $(RM) -f $(CPPSRCARM:.cpp=.s)

# Create output files directory
# all known MS Windows OS define the ComSpec environment variable
$(shell $(MKDIR) -p $(OUTDIR) 2>/dev/null)

# Include the dependency files.
-include $(shell $(MKDIR) -p $(OUTDIR)/dep 2>/dev/null) $(wildcard $(OUTDIR)/dep/*)

# Listing of phony targets.
.PHONY : all build clean clean_list install
