 #####
 # Project: OpenPilot
 #
 #
 # Makefile for OpenPilot project build PiOS and the AP.
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
include $(TOP)/make/boards/$(BOARD_NAME)/board-info.mk

# Target file name (without extension).
TARGET := fw_$(BOARD_NAME)

# Directory for output files (lst, obj, dep, elf, sym, map, hex, bin etc.)
OUTDIR := $(TOP)/build/$(TARGET)

# Set developer code and compile options
# Set to YES to compile for debugging
DEBUG ?= NO

# Include objects that are just nice information to show
STACK_DIAGNOSTICS ?= NO
MIXERSTATUS_DIAGNOSTICS ?= NO
RATEDESIRED_DIAGNOSTICS ?= NO
I2C_WDG_STATS_DIAGNOSTICS ?= NO
DIAG_TASKS ?= NO

#Or just turn on all the above diagnostics. WARNING: This consumes massive amounts of memory.
ALL_DIGNOSTICS ?=NO

# Set to YES to build a FW version that will erase all flash memory
ERASE_FLASH ?= NO
# Set to YES to use the Servo output pins for debugging via scope or logic analyser
ENABLE_DEBUG_PINS ?= NO

# Set to Yes to enable the AUX UART which is mapped on the S1 (Tx) and S2 (Rx) servo outputs
ENABLE_AUX_UART ?= NO

# Set to YES when using Code Sourcery toolchain
CODE_SOURCERY ?= NO

# Remove command is different for Code Sourcery on Windows
ifeq ($(CODE_SOURCERY), YES)
REMOVE_CMD = cs-rm
else
REMOVE_CMD = rm
endif

FLASH_TOOL = OPENOCD

# Optional module and driver defaults
USE_CAMERASTAB ?= YES
USE_COMUSBBRIDGE ?= YES
USE_GPS ?= YES
USE_TXPID ?= YES
USE_I2C ?= NO
USE_ALTITUDE ?= NO
USE_AUTOTUNE ?= YES
TEST_FAULTS ?= NO

# List of optional modules to include
OPTMODULES =
ifeq ($(USE_CAMERASTAB), YES)
OPTMODULES += CameraStab
endif
ifeq ($(USE_COMUSBBRIDGE), YES)
OPTMODULES += ComUsbBridge
endif
ifeq ($(USE_GPS), YES)
OPTMODULES += GPS
endif
ifeq ($(USE_TXPID), YES)
OPTMODULES += TxPID
endif
ifeq ($(USE_ALTITUDE), YES)
ifeq ($(USE_I2C), YES)
OPTMODULES += Altitude
else
$(error "Altitude module (USE_ALTITUDE=YES) requires i2c (USE_I2C=YES)")
endif
endif
ifeq ($(TEST_FAULTS), YES)
OPTMODULES += Fault
endif
ifeq ($(USE_AUTOTUNE), YES)
OPTMODULES += Autotune
endif

# List of mandatory modules to include
MODULES = Attitude Stabilization Actuator ManualControl FirmwareIAP
# Telemetry must be last to grab the optional modules (why?)
MODULES += Telemetry

# Paths
OPSYSTEM = ./System
OPSYSTEMINC = $(OPSYSTEM)/inc
OPUAVTALK = ../UAVTalk
OPUAVTALKINC = $(OPUAVTALK)/inc
OPUAVOBJ = ../UAVObjects
OPUAVOBJINC = $(OPUAVOBJ)/inc
OPTESTS  = ./Tests
OPMODULEDIR = ../Modules
FLIGHTLIB = ../Libraries
FLIGHTLIBINC = $(FLIGHTLIB)/inc
MATHLIB = ../Libraries/math
MATHLIBINC = ../Libraries/math
PIOS = ../PiOS
PIOSINC = $(PIOS)/inc
PIOSSTM32F10X = $(PIOS)/STM32F10x
PIOSCOMMON = $(PIOS)/Common
PIOSBOARDS = $(PIOS)/Boards
APPLIBDIR = $(PIOSSTM32F10X)/Libraries
STMLIBDIR = $(APPLIBDIR)
STMSPDDIR = $(STMLIBDIR)/STM32F10x_StdPeriph_Driver
STMUSBDIR = $(STMLIBDIR)/STM32_USB-FS-Device_Driver
STMSPDSRCDIR = $(STMSPDDIR)/src
STMSPDINCDIR = $(STMSPDDIR)/inc
STMUSBSRCDIR = $(STMUSBDIR)/src
STMUSBINCDIR = $(STMUSBDIR)/inc
CMSISDIR  = $(STMLIBDIR)/CMSIS/Core/CM3
DOSFSDIR  = $(APPLIBDIR)/dosfs
MSDDIR  = $(APPLIBDIR)/msd
RTOSDIR = $(APPLIBDIR)/FreeRTOS
RTOSSRCDIR = $(RTOSDIR)/Source
RTOSINCDIR = $(RTOSSRCDIR)/include
DOXYGENDIR = ../Doc/Doxygen
AHRSBOOTLOADER = ../Bootloaders/AHRS/
AHRSBOOTLOADERINC = $(AHRSBOOTLOADER)/inc
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

OPUAVSYNTHDIR = $(OUTDIR)/../uavobject-synthetics/flight

# List C source files here. (C dependencies are automatically generated.)
# use file-extension c for "c-only"-files

ifndef TESTAPP
## MODULES
SRC += ${foreach MOD, ${OPTMODULES}, ${wildcard ${OPMODULEDIR}/${MOD}/*.c}}
SRC += ${foreach MOD, ${MODULES}, ${wildcard ${OPMODULEDIR}/${MOD}/*.c}}
## OPENPILOT CORE:
SRC += ${OPMODULEDIR}/System/systemmod.c
SRC += $(OPSYSTEM)/coptercontrol.c
SRC += $(OPSYSTEM)/pios_board.c
SRC += $(OPSYSTEM)/alarms.c
SRC += $(OPUAVTALK)/uavtalk.c
SRC += $(OPUAVOBJ)/uavobjectmanager.c
SRC += $(OPUAVOBJ)/eventdispatcher.c
else
## TESTCODE
SRC += $(OPTESTS)/test_common.c
SRC += $(OPTESTS)/$(TESTAPP).c
endif



## UAVOBJECTS
ifndef TESTAPP
SRC += $(OPUAVSYNTHDIR)/accessorydesired.c
SRC += $(OPUAVSYNTHDIR)/objectpersistence.c
SRC += $(OPUAVSYNTHDIR)/gcstelemetrystats.c
SRC += $(OPUAVSYNTHDIR)/flighttelemetrystats.c
SRC += $(OPUAVSYNTHDIR)/faultsettings.c
SRC += $(OPUAVSYNTHDIR)/flightstatus.c
SRC += $(OPUAVSYNTHDIR)/systemstats.c
SRC += $(OPUAVSYNTHDIR)/systemalarms.c
SRC += $(OPUAVSYNTHDIR)/systemsettings.c
SRC += $(OPUAVSYNTHDIR)/stabilizationdesired.c
SRC += $(OPUAVSYNTHDIR)/stabilizationsettings.c
SRC += $(OPUAVSYNTHDIR)/actuatorcommand.c
SRC += $(OPUAVSYNTHDIR)/actuatordesired.c
SRC += $(OPUAVSYNTHDIR)/actuatorsettings.c
SRC += $(OPUAVSYNTHDIR)/accels.c
SRC += $(OPUAVSYNTHDIR)/gyros.c
SRC += $(OPUAVSYNTHDIR)/attitudeactual.c
SRC += $(OPUAVSYNTHDIR)/manualcontrolcommand.c
SRC += $(OPUAVSYNTHDIR)/i2cstats.c
SRC += $(OPUAVSYNTHDIR)/watchdogstatus.c
SRC += $(OPUAVSYNTHDIR)/manualcontrolsettings.c
SRC += $(OPUAVSYNTHDIR)/mixersettings.c
SRC += $(OPUAVSYNTHDIR)/firmwareiapobj.c
SRC += $(OPUAVSYNTHDIR)/attitudesettings.c
SRC += $(OPUAVSYNTHDIR)/camerastabsettings.c
SRC += $(OPUAVSYNTHDIR)/cameradesired.c
SRC += $(OPUAVSYNTHDIR)/gpsposition.c
SRC += $(OPUAVSYNTHDIR)/gpsvelocity.c
SRC += $(OPUAVSYNTHDIR)/gpssettings.c
SRC += $(OPUAVSYNTHDIR)/hwsettings.c
SRC += $(OPUAVSYNTHDIR)/gcsreceiver.c
SRC += $(OPUAVSYNTHDIR)/receiveractivity.c
SRC += $(OPUAVSYNTHDIR)/relaytuningsettings.c
SRC += $(OPUAVSYNTHDIR)/relaytuning.c
SRC += $(OPUAVSYNTHDIR)/taskinfo.c
SRC += $(OPUAVSYNTHDIR)/mixerstatus.c
SRC += $(OPUAVSYNTHDIR)/ratedesired.c
SRC += $(OPUAVSYNTHDIR)/baroaltitude.c
SRC += $(OPUAVSYNTHDIR)/txpidsettings.c

endif

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
SRC += $(PIOSSTM32F10X)/pios_sbus.c
SRC += $(PIOSSTM32F10X)/pios_debug.c
SRC += $(PIOSSTM32F10X)/pios_gpio.c
SRC += $(PIOSSTM32F10X)/pios_exti.c
SRC += $(PIOSSTM32F10X)/pios_rtc.c
SRC += $(PIOSSTM32F10X)/pios_wdg.c
SRC += $(PIOSSTM32F10X)/pios_iap.c
SRC += $(PIOSSTM32F10X)/pios_tim.c
SRC += $(PIOSSTM32F10X)/pios_bl_helper.c

# PIOS USB related files (separated to make code maintenance more easy)
SRC += $(PIOSSTM32F10X)/pios_usb.c
SRC += $(PIOSSTM32F10X)/pios_usbhook.c
SRC += $(PIOSSTM32F10X)/pios_usb_hid.c
SRC += $(PIOSSTM32F10X)/pios_usb_rctx.c
SRC += $(PIOSSTM32F10X)/pios_usb_cdc.c
SRC += $(PIOSSTM32F10X)/pios_usb_hid_istr.c
SRC += $(PIOSSTM32F10X)/pios_usb_hid_pwr.c
SRC += $(OPSYSTEM)/pios_usb_board_data.c
SRC += $(PIOSCOMMON)/pios_usb_desc_hid_cdc.c
SRC += $(PIOSCOMMON)/pios_usb_desc_hid_only.c
SRC += $(PIOSCOMMON)/pios_usb_util.c

## PIOS Hardware (Common)
SRC += $(PIOSCOMMON)/pios_crc.c
SRC += $(PIOSCOMMON)/pios_flashfs_objlist.c
SRC += $(PIOSCOMMON)/pios_flash_jedec.c
SRC += $(PIOSCOMMON)/pios_adxl345.c
SRC += $(PIOSCOMMON)/pios_mpu6000.c
SRC += $(PIOSCOMMON)/pios_com.c
#SRC += $(PIOSCOMMON)/pios_i2c_esc.c
#SRC += $(PIOSCOMMON)/pios_bmp085.c
SRC += $(PIOSCOMMON)/pios_rcvr.c
SRC += $(PIOSCOMMON)/pios_gcsrcvr.c
SRC += $(PIOSCOMMON)/printf-stdarg.c
## Libraries for flight calculations
SRC += $(FLIGHTLIB)/fifo_buffer.c
SRC += $(FLIGHTLIB)/CoordinateConversions.c
SRC += $(FLIGHTLIB)/taskmonitor.c
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
SRC += $(RTOSSRCDIR)/portable/GCC/ARM_CM3/port.c
SRC += $(RTOSSRCDIR)/portable/MemMang/heap_1.c

## Dosfs file system
#SRC += $(DOSFSDIR)/dosfs.c
#SRC += $(DOSFSDIR)/dfs_sdcard.c

## PyMite files
#SRC += $(wildcard ${PYMITEVM}/*.c)
#SRC += $(wildcard ${PYMITEPLAT}/*.c)
#SRC += $(OUTDIR)/pmlib_img.c
#SRC += $(OUTDIR)/pmlib_nat.c
#SRC += $(OUTDIR)/pmlibusr_img.c
#SRC += $(OUTDIR)/pmlibusr_nat.c

## Mass Storage Device
#SRC += $(MSDDIR)/msd.c
#SRC += $(MSDDIR)/msd_bot.c
#SRC += $(MSDDIR)/msd_desc.c
#SRC += $(MSDDIR)/msd_memory.c
#SRC += $(MSDDIR)/msd_scsi.c
#SRC += $(MSDDIR)/msd_scsi_data.c

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
ASRC = $(PIOSSTM32F10X)/startup_stm32f10x_$(MODEL)$(MODEL_SUFFIX).S

# List Assembler source files here which must be assembled in ARM-Mode..
ASRCARM =

# List any extra directories to look for include files here.
#    Each directory must be seperated by a space.
EXTRAINCDIRS  =  $(OPSYSTEM)
EXTRAINCDIRS  += $(OPSYSTEMINC)
EXTRAINCDIRS  += $(OPUAVTALK)
EXTRAINCDIRS  += $(OPUAVTALKINC)
EXTRAINCDIRS  += $(OPUAVOBJ)
EXTRAINCDIRS  += $(OPUAVOBJINC)
EXTRAINCDIRS  += $(OPUAVSYNTHDIR)
EXTRAINCDIRS  += $(PIOS)
EXTRAINCDIRS  += $(PIOSINC)
EXTRAINCDIRS  += $(FLIGHTLIBINC)
EXTRAINCDIRS  += $(MATHLIBINC)
EXTRAINCDIRS  += $(PIOSSTM32F10X)
EXTRAINCDIRS  += $(PIOSCOMMON)
EXTRAINCDIRS  += $(PIOSBOARDS)
EXTRAINCDIRS  += $(STMSPDINCDIR)
EXTRAINCDIRS  += $(STMUSBINCDIR)
EXTRAINCDIRS  += $(CMSISDIR)
EXTRAINCDIRS  += $(DOSFSDIR)
EXTRAINCDIRS  += $(MSDDIR)
EXTRAINCDIRS  += $(RTOSINCDIR)
EXTRAINCDIRS  += $(APPLIBDIR)
EXTRAINCDIRS  += $(RTOSSRCDIR)/portable/GCC/ARM_CM3
EXTRAINCDIRS  += $(AHRSBOOTLOADERINC)
EXTRAINCDIRS  += $(PYMITEINC)
EXTRAINCDIRS  += $(HWDEFSINC)

EXTRAINCDIRS += ${foreach MOD, ${OPTMODULES} ${MODULES}, ${OPMODULEDIR}/${MOD}/inc} ${OPMODULEDIR}/System/inc


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
OPT = 1
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
CDEFS = -DSTM32F10X_$(MODEL)
CDEFS += -DUSE_STDPERIPH_DRIVER
CDEFS += -DUSE_$(BOARD)
ifeq ($(ENABLE_DEBUG_PINS), YES)
CDEFS += -DPIOS_ENABLE_DEBUG_PINS
endif
ifeq ($(ENABLE_AUX_UART), YES)
CDEFS += -DPIOS_ENABLE_AUX_UART
endif
ifeq ($(ERASE_FLASH), YES)
CDEFS += -DERASE_FLASH
endif

ifneq ($(USE_GPS), NO)
CDEFS += -DUSE_GPS
endif

ifeq ($(USE_I2C), YES)
CDEFS += -DUSE_I2C
endif

# Declare all non-optional modules as built-in to force inclusion
CDEFS += ${foreach MOD, ${MODULES}, -DMODULE_$(MOD)_BUILTIN }

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
CFLAGS += -DDEBUG
endif

#The following Makefile command, ifneq (, $(filter) $(A), $(B) $(C)) is equivalent 
# to the pseudocode `if(A== B || A==C)`
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

CFLAGS += -g$(DEBUGF)
CFLAGS += -O$(OPT)
CFLAGS += -mcpu=$(MCU)
CFLAGS += $(CDEFS)
CFLAGS += $(patsubst %,-I%,$(EXTRAINCDIRS)) -I.

#CFLAGS += -fno-cprop-registers -fno-defer-pop -fno-guess-branch-probability -fno-section-anchors
#CFLAGS += -fno-if-conversion -fno-if-conversion2 -fno-ipa-pure-const -fno-ipa-reference -fno-merge-constants
#CFLAGS += -fno-split-wide-types -fno-tree-ccp -fno-tree-ch -fno-tree-copy-prop -fno-tree-copyrename
#CFLAGS += -fno-tree-dce -fno-tree-dominator-opts -fno-tree-dse -fno-tree-fre -fno-tree-sink -fno-tree-sra
#CFLAGS += -fno-tree-ter
#CFLAGS += -g$(DEBUGF) -DDEBUG

CFLAGS += -mapcs-frame
CFLAGS += -fomit-frame-pointer
ifeq ($(CODE_SOURCERY), YES)
CFLAGS += -fpromote-loop-indices
endif

CFLAGS += -Wall
CFLAGS += -Werror
CFLAGS += -ffunction-sections -fdata-sections
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
LDFLAGS += $(patsubst %,-L%,$(EXTRA_LIBDIRS))
LDFLAGS += -lc
LDFLAGS += $(patsubst %,-l%,$(EXTRA_LIBS))
LDFLAGS += $(MATH_LIB)
LDFLAGS += -lc -lgcc

# Set linker-script name depending on selected submodel name
LDFLAGS += -T$(LINKERSCRIPTPATH)/link_$(BOARD)_memory.ld
LDFLAGS += -T$(LINKERSCRIPTPATH)/link_$(BOARD)_sections.ld

# Define programs and commands.
REMOVE  = $(REMOVE_CMD) -f
PYTHON  = python

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

# Generate code for PyMite
#$(OUTDIR)/pmlib_img.c $(OUTDIR)/pmlib_nat.c $(OUTDIR)/pmlibusr_img.c $(OUTDIR)/pmlibusr_nat.c $(OUTDIR)/pmfeatures.h: $(wildcard $(PYMITELIB)/*.py) $(wildcard $(PYMITEPLAT)/*.py) $(wildcard $(FLIGHTPLANLIB)/*.py) $(wildcard $(FLIGHTPLANS)/*.py)
#	@echo $(MSG_PYMITEINIT) $(call toprel, $@)
#	@$(PYTHON) $(PYMITETOOLS)/pmImgCreator.py -f $(PYMITEPLAT)/pmfeatures.py -c -s --memspace=flash -o $(OUTDIR)/pmlib_img.c --native-file=$(OUTDIR)/pmlib_nat.c $(PYMITELIB)/list.py $(PYMITELIB)/dict.py $(PYMITELIB)/__bi.py $(PYMITELIB)/sys.py $(PYMITELIB)/string.py $(wildcard $(FLIGHTPLANLIB)/*.py)
#	@$(PYTHON) $(PYMITETOOLS)/pmGenPmFeatures.py $(PYMITEPLAT)/pmfeatures.py > $(OUTDIR)/pmfeatures.h
#	@$(PYTHON) $(PYMITETOOLS)/pmImgCreator.py -f $(PYMITEPLAT)/pmfeatures.py -c -u -o $(OUTDIR)/pmlibusr_img.c --native-file=$(OUTDIR)/pmlibusr_nat.c $(FLIGHTPLANS)/test.py

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
	$(V1) $(REMOVE) $(wildcard $(OUTDIR)/*.c)
	$(V1) $(REMOVE) $(wildcard $(OUTDIR)/*.h)
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
