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


# Set developer code and compile options
# Set to YES to compile for debugging
DEBUG ?= YES

# Set to YES to use the Servo output pins for debugging via scope or logic analyser
ENABLE_DEBUG_PINS ?= NO

# Set to Yes to enable the AUX UART which is mapped on the S1 (Tx) and S2 (Rx) servo outputs
ENABLE_AUX_UART ?= NO

#
USE_BOOTLOADER ?= NO


# Set to YES when using Code Sourcery toolchain
CODE_SOURCERY ?= YES

# Toolchain prefix (i.e arm-elf- -> arm-elf-gcc.exe)
TCHAIN_PREFIX ?= arm-none-eabi-

# Remove command is different for Code Sourcery on Windows
ifeq ($(CODE_SOURCERY), YES)
REMOVE_CMD = cs-rm
else
REMOVE_CMD = rm
endif

FLASH_TOOL = OPENOCD

# YES enables -mthumb option to flags for source-files listed
# in SRC and CPPSRC
USE_THUMB_MODE = YES

# List of modules to include
MODULES =  Telemetry GPS ManualControl Altitude AHRSComms Stabilization/simple/Stabilization Watchdog
#matrix based actuator mixer
#MODULES += Actuator/matrix/Actuator
#original actuator with separate mixers for fixed wing, heli and VTOL
MODULES += Actuator

#MODULES = Telemetry Example
#MODULES = Telemetry MK/MKSerial
#MODULES = Telemetry
#MODULES += Osd/OsdEtStd


# MCU name, submodel and board
# - MCU used for compiler-option (-mcpu)
# - MODEL used for linker-script name (-T) and passed as define
# - BOARD just passed as define (optional)
MCU      = cortex-m3
CHIP     = STM32F103RET
BOARD    = STM3210E_OP
MODEL	 = HD
ifeq ($(USE_BOOTLOADER), YES)
BOOT_MODEL    = $(MODEL)_BL

else
BOOT_MODEL    = $(MODEL)_NB
endif

# Directory for output files (lst, obj, dep, elf, sym, map, hex, bin etc.)
OUTDIR = Build

# Target file name (without extension).
TARGET = OpenPilot

# Paths
OPSYSTEM = ./System
OPSYSTEMINC = $(OPSYSTEM)/inc
OPUAVTALK = ./UAVTalk
OPUAVTALKINC = $(OPUAVTALK)/inc
OPUAVOBJ = ./UAVObjects
OPUAVOBJINC = $(OPUAVOBJ)/inc
OPTESTS  = ./Tests
OPMODULEDIR = ./Modules
FLIGHTLIB = ../Libraries
FLIGHTLIBINC = ../Libraries/inc
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

# List C source files here. (C dependencies are automatically generated.)
# use file-extension c for "c-only"-files

MODNAMES = $(notdir ${MODULES})

ifndef TESTAPP
## MODULES
SRC += ${foreach MOD, ${MODULES}, ${wildcard ${OPMODULEDIR}/${MOD}/*.c}}
SRC += ${OUTDIR}/InitMods.c
## OPENPILOT CORE:
SRC += ${OPMODULEDIR}/System/systemmod.c
SRC += $(OPSYSTEM)/openpilot.c
SRC += $(OPSYSTEM)/pios_board.c
SRC += $(OPSYSTEM)/alarms.c
SRC += $(OPUAVTALK)/uavtalk.c
SRC += $(OPUAVOBJ)/uavobjectmanager.c
SRC += $(OPUAVOBJ)/uavobjectsinit.c
SRC += $(OPUAVOBJ)/eventdispatcher.c
else
## TESTCODE
SRC += $(OPTESTS)/test_common.c
SRC += $(OPTESTS)/$(TESTAPP).c
endif



## UAVOBJECTS
ifndef TESTAPP
SRC += $(OPUAVOBJ)/exampleobject1.c
SRC += $(OPUAVOBJ)/exampleobject2.c
SRC += $(OPUAVOBJ)/examplesettings.c
SRC += $(OPUAVOBJ)/objectpersistence.c
SRC += $(OPUAVOBJ)/positionactual.c
SRC += $(OPUAVOBJ)/gpsposition.c
SRC += $(OPUAVOBJ)/gpstime.c
SRC += $(OPUAVOBJ)/gpssatellites.c
SRC += $(OPUAVOBJ)/gcstelemetrystats.c
SRC += $(OPUAVOBJ)/flighttelemetrystats.c
SRC += $(OPUAVOBJ)/systemstats.c
SRC += $(OPUAVOBJ)/systemalarms.c
SRC += $(OPUAVOBJ)/systemsettings.c
SRC += $(OPUAVOBJ)/telemetrysettings.c
SRC += $(OPUAVOBJ)/actuatorcommand.c
SRC += $(OPUAVOBJ)/actuatordesired.c
SRC += $(OPUAVOBJ)/actuatorsettings.c
SRC += $(OPUAVOBJ)/manualcontrolcommand.c
SRC += $(OPUAVOBJ)/manualcontrolsettings.c
SRC += $(OPUAVOBJ)/attitudedesired.c
SRC += $(OPUAVOBJ)/stabilizationsettings.c
SRC += $(OPUAVOBJ)/ahrsstatus.c
SRC += $(OPUAVOBJ)/baroaltitude.c
SRC += $(OPUAVOBJ)/ahrscalibration.c
SRC += $(OPUAVOBJ)/attitudeactual.c
SRC += $(OPUAVOBJ)/ahrssettings.c
SRC += $(OPUAVOBJ)/flightbatterystate.c
SRC += $(OPUAVOBJ)/attituderaw.c
SRC += $(OPUAVOBJ)/homelocation.c
SRC += $(OPUAVOBJ)/attitudesettings.c
SRC += $(OPUAVOBJ)/vtolsettings.c
SRC += $(OPUAVOBJ)/vtolstatus.c
SRC += $(OPUAVOBJ)/mixersettings.c
SRC += $(OPUAVOBJ)/mixerstatus.c
#SRC += $(OPUAVOBJ)/lesstabilizationsettings.c
SRC += $(OPUAVOBJ)/positiondesired.c
SRC += $(OPUAVOBJ)/velocitydesired.c
SRC += $(OPUAVOBJ)/velocityactual.c
SRC += $(OPUAVOBJ)/guidancesettings.c
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
SRC += $(PIOSSTM32F10X)/pios_spektrum.c
SRC += $(PIOSSTM32F10X)/pios_debug.c
SRC += $(PIOSSTM32F10X)/pios_gpio.c
SRC += $(PIOSSTM32F10X)/pios_exti.c
SRC += $(PIOSSTM32F10X)/pios_wdg.c

# PIOS USB related files (seperated to make code maintenance more easy)
SRC += $(PIOSSTM32F10X)/pios_usb_hid.c
SRC += $(PIOSSTM32F10X)/pios_usb_hid_desc.c
#SRC += $(PIOSSTM32F10X)/pios_usb_hid_endp.c
SRC += $(PIOSSTM32F10X)/pios_usb_hid_istr.c
SRC += $(PIOSSTM32F10X)/pios_usb_hid_prop.c
SRC += $(PIOSSTM32F10X)/pios_usb_hid_pwr.c

## PIOS Hardware (Common)
SRC += $(PIOSCOMMON)/pios_sdcard.c
SRC += $(PIOSCOMMON)/pios_com.c
SRC += $(PIOSCOMMON)/pios_bmp085.c
SRC += $(PIOSCOMMON)/pios_opahrs.c
SRC += $(PIOSCOMMON)/pios_opahrs_proto.c
SRC += $(PIOSCOMMON)/printf-stdarg.c

## Libraries for flight calculations
SRC += $(FLIGHTLIB)/buffer.c
SRC += $(FLIGHTLIB)/WorldMagModel.c
SRC += $(FLIGHTLIB)/CoordinateConversions.c

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
SRC += $(RTOSSRCDIR)/portable/MemMang/heap_2.c

## Dosfs file system
SRC += $(DOSFSDIR)/dosfs.c
SRC += $(DOSFSDIR)/dfs_sdcard.c

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
ASRC = $(PIOSSTM32F10X)/startup_stm32f10x_$(MODEL)_OP.S

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
EXTRAINCDIRS  += $(PIOS)
EXTRAINCDIRS  += $(PIOSINC)
EXTRAINCDIRS  += $(FLIGHTLIBINC)
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

EXTRAINCDIRS += ${foreach MOD, ${MODULES}, Modules/${MOD}/inc} ${OPMODULEDIR}/System/inc


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
ifeq ($(USE_BOOTLOADER), YES)
CDEFS += -DUSE_BOOTLOADER
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

ifeq ($(DEBUG),YES)
CFLAGS = -g$(DEBUGF) -DDEBUG
endif

CFLAGS += -O$(OPT)
CFLAGS += -mcpu=$(MCU) -mthumb
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
ASFLAGS  = -mcpu=$(MCU) -mthumb -I. -x assembler-with-cpp
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
LDFLAGS +=-T$(LINKERSCRIPTPATH)/link_stm32f10x_$(BOOT_MODEL).ld

# ---------------------------------------------------------------------------
# Options for OpenOCD flash-programming
# see openocd.pdf/openocd.texi for further information
#
OOCD_LOADFILE+=$(OUTDIR)/$(TARGET).elf
# if OpenOCD is in the $PATH just set OOCD_EXE=openocd
OOCD_EXE=openocd
# debug level
OOCD_CL=-d0
# interface and board/target settings (using the OOCD target-library here)
UNAME := $(shell uname)
ifeq ($(UNAME), Darwin)
  OOCD_CL+=-f ../Project/OpenOCD/floss-jtag.openpilot.osx.cfg -f ../Project/OpenOCD/stm32.cfg
else
  OOCD_CL+=-f ../Project/OpenOCD/floss-jtag-revb.cfg -f ../Project/OpenOCD/stm32.cfg
endif
# initialize
OOCD_CL+=-c init
# show the targets
OOCD_CL+=-c targets
# commands to prepare flash-write
OOCD_CL+= -c "reset halt"
# flash erase
OOCD_CL+=-c "stm32x mass_erase 0"
# flash-write
OOCD_CL+=-c "flash write_image $(OOCD_LOADFILE)"
# Verify
OOCD_CL+=-c "verify_image $(OOCD_LOADFILE)"
# reset target
OOCD_CL+=-c "reset run"
# terminate OOCD after programming
OOCD_CL+=-c shutdown
# ---------------------------------------------------------------------------


# Define programs and commands.
CC      = $(TCHAIN_PREFIX)gcc
CPP     = $(TCHAIN_PREFIX)g++
AR      = $(TCHAIN_PREFIX)ar
OBJCOPY = $(TCHAIN_PREFIX)objcopy
OBJDUMP = $(TCHAIN_PREFIX)objdump
SIZE    = $(TCHAIN_PREFIX)size
NM      = $(TCHAIN_PREFIX)nm
REMOVE  = $(REMOVE_CMD) -f
###SHELL   = sh
###COPY    = cp



# Define Messages
# English
MSG_ERRORS_NONE = Errors: none
MSG_BEGIN = ${quote}-------- begin (mode: $(RUN_MODE)) --------${quote}
MSG_END = ${quote}--------  end  --------${quote}
MSG_MODINIT = ${quote}**** Generating ModInit.c${quote}
MSG_SIZE_BEFORE = ${quote}Size before:${quote}
MSG_SIZE_AFTER = ${quote}Size after build:${quote}
MSG_LOAD_FILE = ${quote}Creating load file:${quote}
MSG_EXTENDED_LISTING = ${quote}Creating Extended Listing/Disassembly:${quote}
MSG_SYMBOL_TABLE = ${quote}Creating Symbol Table:${quote}
MSG_LINKING = ${quote}**** Linking :${quote}
MSG_COMPILING = ${quote}**** Compiling C :${quote}
MSG_COMPILING_ARM = ${quote}**** Compiling C (ARM-only):${quote}
MSG_COMPILINGCPP = ${quote}Compiling C++ :${quote}
MSG_COMPILINGCPP_ARM = ${quote}Compiling C++ (ARM-only):${quote}
MSG_ASSEMBLING = ${quote}**** Assembling:${quote}
MSG_ASSEMBLING_ARM = ${quote}****Assembling (ARM-only):${quote}
MSG_CLEANING = ${quote}Cleaning project:${quote}
MSG_FORMATERROR = ${quote}Can not handle output-format${quote}
MSG_ASMFROMC = ${quote}Creating asm-File from C-Source:${quote}
MSG_ASMFROMC_ARM = ${quote}Creating asm-File from C-Source (ARM-only):${quote}

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

elf: $(OUTDIR)/$(TARGET).elf
lss: $(OUTDIR)/$(TARGET).lss
sym: $(OUTDIR)/$(TARGET).sym
hex: $(OUTDIR)/$(TARGET).hex
bin: $(OUTDIR)/$(TARGET).bin

# Default target.
#all: begin gccversion sizebefore build sizeafter finished end
all: begin gccversion build sizeafter finished end

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

# Test if quotes are needed for the echo-command
result = ${shell echo "test"}
ifeq (${result}, test)
	quote = '
else
	quote =
endif

${OUTDIR}/InitMods.c: Makefile
	@echo ${MSG_MODINIT}
	@echo ${quote}// Autogenerated file${quote} > ${OUTDIR}/InitMods.c
	@echo ${quote}${foreach MOD, ${MODNAMES}, extern unsigned int ${MOD}Initialize(void);}${quote}  >> ${OUTDIR}/InitMods.c
	@echo ${quote}void InitModules() {${quote} >> ${OUTDIR}/InitMods.c
	@echo ${quote}${foreach MOD, ${MODNAMES}, ${MOD}Initialize();}${quote}  >> ${OUTDIR}/InitMods.c
	@echo ${quote}}${quote} >> ${OUTDIR}/InitMods.c

# Eye candy.
begin:
##	@echo
	@echo $(MSG_BEGIN)

finished:
##	@echo $(MSG_ERRORS_NONE)

end:
	@echo $(MSG_END)
##	@echo

# Display sizes of sections.
ELFSIZE = $(SIZE) -A  $(OUTDIR)/$(TARGET).elf
##ELFSIZE = $(SIZE) --format=Berkeley --common $(OUTDIR)/$(TARGET).elf
sizebefore:
#	@if [ -f  $(OUTDIR)/$(TARGET).elf ]; then echo; echo $(MSG_SIZE_BEFORE); $(ELFSIZE); echo; fi

sizeafter:
#	@if [ -f  $(OUTDIR)/$(TARGET).elf ]; then echo; echo $(MSG_SIZE_AFTER); $(ELFSIZE); echo; fi
	@echo $(MSG_SIZE_AFTER)
	$(ELFSIZE)

# Display compiler version information.
gccversion :
	@$(CC) --version
#	@echo $(ALLOBJ)

# Program the device.
ifeq ($(USE_BOOTLOADER), YES)
# Program the device with OP Upload Tool".
program: $(OUTDIR)/$(TARGET).bin
	@echo ${quote}Programming with OP Upload Tool${quote}
	../../ground/src/experimental/upload-build-desktop/debug/OPUploadTool -d 0 -p $(OUTDIR)/$(TARGET).bin
else
ifeq ($(FLASH_TOOL),OPENOCD)
# Program the device with Dominic Rath's OPENOCD in "batch-mode", needs cfg and "reset-script".
program: $(OUTDIR)/$(TARGET).elf
	@echo ${quote}Programming with OPENOCD${quote}
	$(OOCD_EXE) $(OOCD_CL)
endif
endif

# Create final output file (.hex) from ELF output file.
%.hex: %.elf
##	@echo
	@echo $(MSG_LOAD_FILE) $@
	$(OBJCOPY) -O ihex $< $@

# Create final output file (.bin) from ELF output file.
%.bin: %.elf
##	@echo
	@echo $(MSG_LOAD_FILE) $@
	$(OBJCOPY) -O binary $< $@

# Create extended listing file/disassambly from ELF output file.
# using objdump testing: option -C
%.lss: %.elf
##	@echo
	@echo $(MSG_EXTENDED_LISTING) $@
	$(OBJDUMP) -h -S -C -r $< > $@
#	$(OBJDUMP) -x -S $< > $@

# Create a symbol table from ELF output file.
%.sym: %.elf
##	@echo
	@echo $(MSG_SYMBOL_TABLE) $@
	$(NM) -n $< > $@

# Link: create ELF output file from object files.
.SECONDARY : $(TARGET).elf
.PRECIOUS : $(ALLOBJ)
%.elf:  $(ALLOBJ)
	@echo $(MSG_LINKING) $@
# use $(CC) for C-only projects or $(CPP) for C++-projects:
	$(CC) $(THUMB) $(CFLAGS) $(ALLOBJ) --output $@ $(LDFLAGS)
#	$(CPP) $(THUMB) $(CFLAGS) $(ALLOBJ) --output $@ $(LDFLAGS)


# Assemble: create object files from assembler source files.
define ASSEMBLE_TEMPLATE
$(OUTDIR)/$(notdir $(basename $(1))).o : $(1)
##	@echo
	@echo $(MSG_ASSEMBLING) $$<  to  $$@
	$(CC) -c $(THUMB) $$(ASFLAGS) $$< -o $$@
endef
$(foreach src, $(ASRC), $(eval $(call ASSEMBLE_TEMPLATE, $(src))))

# Assemble: create object files from assembler source files. ARM-only
define ASSEMBLE_ARM_TEMPLATE
$(OUTDIR)/$(notdir $(basename $(1))).o : $(1)
##	@echo
	@echo $(MSG_ASSEMBLING_ARM) $$<  to  $$@
	$(CC) -c $$(ASFLAGS) $$< -o $$@
endef
$(foreach src, $(ASRCARM), $(eval $(call ASSEMBLE_ARM_TEMPLATE, $(src))))


# Compile: create object files from C source files.
define COMPILE_C_TEMPLATE
$(OUTDIR)/$(notdir $(basename $(1))).o : $(1)
##	@echo
	@echo $(MSG_COMPILING) $$<  to  $$@
	$(CC) -c $(THUMB) $$(CFLAGS) $$(CONLYFLAGS) $$< -o $$@
endef
$(foreach src, $(SRC), $(eval $(call COMPILE_C_TEMPLATE, $(src))))

# Compile: create object files from C source files. ARM-only
define COMPILE_C_ARM_TEMPLATE
$(OUTDIR)/$(notdir $(basename $(1))).o : $(1)
##	@echo
	@echo $(MSG_COMPILING_ARM) $$<  to  $$@
	$(CC) -c $$(CFLAGS) $$(CONLYFLAGS) $$< -o $$@
endef
$(foreach src, $(SRCARM), $(eval $(call COMPILE_C_ARM_TEMPLATE, $(src))))


# Compile: create object files from C++ source files.
define COMPILE_CPP_TEMPLATE
$(OUTDIR)/$(notdir $(basename $(1))).o : $(1)
##	@echo
	@echo $(MSG_COMPILINGCPP) $$<  to  $$@
	$(CC) -c $(THUMB) $$(CFLAGS) $$(CPPFLAGS) $$< -o $$@
endef
$(foreach src, $(CPPSRC), $(eval $(call COMPILE_CPP_TEMPLATE, $(src))))

# Compile: create object files from C++ source files. ARM-only
define COMPILE_CPP_ARM_TEMPLATE
$(OUTDIR)/$(notdir $(basename $(1))).o : $(1)
##	@echo
	@echo $(MSG_COMPILINGCPP_ARM) $$<  to  $$@
	$(CC) -c $$(CFLAGS) $$(CPPFLAGS) $$< -o $$@
endef
$(foreach src, $(CPPSRCARM), $(eval $(call COMPILE_CPP_ARM_TEMPLATE, $(src))))


# Compile: create assembler files from C source files. ARM/Thumb
$(SRC:.c=.s) : %.s : %.c
	@echo $(MSG_ASMFROMC) $< to $@
	$(CC) $(THUMB) -S $(CFLAGS) $(CONLYFLAGS) $< -o $@

# Compile: create assembler files from C source files. ARM only
$(SRCARM:.c=.s) : %.s : %.c
	@echo $(MSG_ASMFROMC_ARM) $< to $@
	$(CC) -S $(CFLAGS) $(CONLYFLAGS) $< -o $@

# Generate Doxygen documents
docs:
	doxygen  $(DOXYGENDIR)/doxygen.cfg

# Target: clean project.
clean: begin clean_list finished end

clean_list :
##	@echo
	@echo $(MSG_CLEANING)
	$(REMOVE) $(OUTDIR)/$(TARGET).map
	$(REMOVE) $(OUTDIR)/$(TARGET).elf
	$(REMOVE) $(OUTDIR)/$(TARGET).hex
	$(REMOVE) $(OUTDIR)/$(TARGET).bin
	$(REMOVE) $(OUTDIR)/$(TARGET).sym
	$(REMOVE) $(OUTDIR)/$(TARGET).lss
	$(REMOVE) $(ALLOBJ)
	$(REMOVE) $(LSTFILES)
	$(REMOVE) $(DEPFILES)
	$(REMOVE) $(SRC:.c=.s)
	$(REMOVE) $(SRCARM:.c=.s)
	$(REMOVE) $(CPPSRC:.cpp=.s)
	$(REMOVE) $(CPPSRCARM:.cpp=.s)


# Create output files directory
# all known MS Windows OS define the ComSpec environment variable
ifdef ComSpec
$(shell md $(OUTDIR) 2>NUL)
else
$(shell mkdir $(OUTDIR) 2>/dev/null)
endif

# Include the dependency files.
ifdef ComSpec
-include $(shell md $(OUTDIR)\dep 2>NUL) $(wildcard $(OUTDIR)/dep/*)
else
-include $(shell mkdir $(OUTDIR) 2>/dev/null) $(shell mkdir $(OUTDIR)/dep 2>/dev/null) $(wildcard $(OUTDIR)/dep/*)
endif



# Listing of phony targets.
.PHONY : all begin finish end sizebefore sizeafter gccversion \
build elf hex bin lss sym clean clean_list program

