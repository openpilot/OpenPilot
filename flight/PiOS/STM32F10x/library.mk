#
# Rules to (help) build the F1xx device support.
#

# Directory containing this makefile
PIOS_DEVLIB		:= $(dir $(lastword $(MAKEFILE_LIST)))

# Linker scripts path (contains board-specific scripts)
LINKER_SCRIPTS_PATH	=  $(PIOS_DEVLIB)

# Compiler options implied by the F10x
CDEFS			+= -DSTM32F10X -DSTM32F10X_$(MODEL)
CDEFS 			+= -DUSE_STDPERIPH_DRIVER
CDEFS			+= -DARM_MATH_CM3

ARCHFLAGS		+= -mcpu=cortex-m3

# Board-specific startup files
ASRC			+= $(PIOS_DEVLIB)/startup_stm32f10x_$(MODEL)$(MODEL_SUFFIX).S

# PIOS device library source and includes
SRC			+= $(wildcard $(PIOS_DEVLIB)*.c)

# CMSIS for the F1
CMSIS_DIR		= $(PIOS_DEVLIB)/Libraries/CMSIS/Core/CM3
SRC			+= $(CMSIS_DIR)/core_cm3.c
SRC			+= $(CMSIS_DIR)/system_stm32f10x.c
EXTRAINCDIRS		+= $(CMSIS_DIR)

# ST Peripheral library
PERIPHLIB		=  $(PIOS_DEVLIB)/Libraries/STM32F10x_StdPeriph_Driver
SRC			+= $(wildcard $(PERIPHLIB)/src/*.c)
EXTRAINCDIRS		+= $(PERIPHLIB)/inc

# ST USB Device library
USBDEVLIB		=  $(PIOS_DEVLIB)/Libraries/STM32_USB-FS-Device_Driver
SRC			+= $(wildcard $(USBDEVLIB)/src/*.c)
EXTRAINCDIRS		+= $(USBDEVLIB)/inc

#
# FreeRTOS
#
# If the application has included the generic FreeRTOS support, then add in
# the device-specific pieces of the code.
#
ifneq ($(FREERTOS_DIR),)
    FREERTOS_PORTDIR	:= $(PIOS_DEVLIB)/Libraries/FreeRTOS/Source
    SRC			+= $(wildcard $(FREERTOS_PORTDIR)/portable/GCC/ARM_CM3/*.c)
    SRC			+= $(wildcard $(FREERTOS_PORTDIR)/portable/MemMang/heap_1.c)
    EXTRAINCDIRS	+= $(FREERTOS_PORTDIR)/portable/GCC/ARM_CM3
endif
