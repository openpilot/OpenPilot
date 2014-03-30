#
# Rules to (help) build the F10x device support.
#

# Directory containing this makefile
PIOS_DEVLIB		:= $(dir $(lastword $(MAKEFILE_LIST)))

# Linker scripts path (contains board-specific scripts)
LINKER_SCRIPTS_PATH	=  $(PIOS_DEVLIB)

# Compiler options implied by the F10x
CDEFS			+= -DSTM32F0 
CDEFS 			+= -DUSE_STDPERIPH_DRIVER
CDEFS			+= -DARM_MATH_CM0
CDEFS			+= -DHSE_VALUE=$(OSCILLATOR_FREQ)
ARCHFLAGS		+= -mcpu=cortex-m0 --specs=nano.specs

# Board-specific startup files
ASRC			+= $(PIOS_DEVLIB)startup_stm32f0$(MODEL)$(MODEL_SUFFIX).S

# PIOS device library source and includes
SRC			+= $(sort $(wildcard $(PIOS_DEVLIB)*.c))
EXTRAINCDIRS		+= $(PIOS_DEVLIB)inc

# CMSIS for the F0
include $(PIOSCOMMON)/libraries/CMSIS/library.mk

CMSIS_DEVICEDIR		= $(PIOS_DEVLIB)libraries/CMSIS/Device/ST/STM32F0xx/
CMSIS_DIR		= $(PIOS_DEVLIB)/libraries/CMSIS/Include
EXTRAINCDIRS		+= $(CMSIS_DEVICEDIR)/Include
EXTRAINCDIRS		+= $(CMSIS_DIR)
# ST Peripheral library
PERIPHLIB		=  $(PIOS_DEVLIB)libraries/STM32F0xx_StdPeriph_Driver
SRC			+= $(sort $(wildcard $(PERIPHLIB)/src/*.c))
EXTRAINCDIRS		+= $(PERIPHLIB)/inc

#
# FreeRTOS
#
# If the application has included the generic FreeRTOS support, then add in
# the device-specific pieces of the code.
#
ifneq ($(FREERTOS_DIR),)
    FREERTOS_PORTDIR	:= $(FREERTOS_DIR)
    SRC			+= $(sort $(wildcard $(FREERTOS_PORTDIR)/portable/GCC/ARM_CM0/*.c))
    SRC			+= $(sort $(wildcard $(FREERTOS_DIR)/portable/MemMang/heap_1.c))
    EXTRAINCDIRS	+= $(FREERTOS_PORTDIR)/portable/GCC/ARM_CM0
endif
