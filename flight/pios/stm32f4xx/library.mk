#
# Rules to (help) build the F4xx device support.
#

# Directory containing this makefile
PIOS_DEVLIB		:= $(dir $(lastword $(MAKEFILE_LIST)))

# Hardcoded linker script names for now
LINKER_SCRIPTS_APP	=  $(PIOS_DEVLIB)link_stm32f4xx_fw_memory.ld \
			   $(PIOS_DEVLIB)link_stm32f4xx_sections.ld
LINKER_SCRIPTS_BL	=  $(PIOS_DEVLIB)link_stm32f4xx_bl_memory.ld \
			   $(PIOS_DEVLIB)link_stm32f4xx_sections.ld
# _compat linker script are aimed at bootloader updater to guarantee to be compatible with earlier bootloaders.
LINKER_SCRIPTS_COMPAT	=  $(PIOS_DEVLIB)link_stm32f4xx_fw_memory.ld \
			   $(PIOS_DEVLIB)link_stm32f4xx_sections_compat.ld

# Compiler options implied by the F4xx
CDEFS			+= -DSTM32F4XX
CDEFS			+= -DSYSCLK_FREQ=$(SYSCLK_FREQ)
CDEFS			+= -DHSE_VALUE=$(OSCILLATOR_FREQ)
CDEFS 			+= -DUSE_STDPERIPH_DRIVER
CDEFS			+= -DARM_MATH_CM4 -D__FPU_PRESENT=1
ARCHFLAGS		+= -mcpu=cortex-m4 -march=armv7e-m -mfpu=fpv4-sp-d16 -mfloat-abi=hard

# PIOS device library source and includes
SRC			+= $(sort $(wildcard $(PIOS_DEVLIB)*.c))
EXTRAINCDIRS		+= $(PIOS_DEVLIB)inc

# CMSIS for the F4
include $(PIOSCOMMON)/libraries/CMSIS/library.mk
CMSIS_DEVICEDIR	:= $(PIOS_DEVLIB)libraries/CMSIS2/Device/ST/STM32F4xx
SRC			+= $(sort $(wildcard $(CMSIS_DEVICEDIR)/Source/$(BOARD_NAME)/*.c))
EXTRAINCDIRS		+= $(CMSIS_DEVICEDIR)/Include

# ST Peripheral library
PERIPHLIB		=  $(PIOS_DEVLIB)libraries/STM32F4xx_StdPeriph_Driver
SRC			+= $(sort $(wildcard $(PERIPHLIB)/src/*.c))
EXTRAINCDIRS		+= $(PERIPHLIB)/inc

# ST USB OTG library
USBOTGLIB		=  $(PIOS_DEVLIB)libraries/STM32_USB_OTG_Driver
USBOTGLIB_SRC		=  usb_core.c usb_dcd.c usb_dcd_int.c
SRC			+= $(addprefix $(USBOTGLIB)/src/,$(USBOTGLIB_SRC))
EXTRAINCDIRS		+= $(USBOTGLIB)/inc

# ST USB Device library
USBDEVLIB		=  $(PIOS_DEVLIB)libraries/STM32_USB_Device_Library
SRC			+= $(sort $(wildcard $(USBDEVLIB)/Core/src/*.c))
EXTRAINCDIRS		+= $(USBDEVLIB)/Core/inc

#
# FreeRTOS
#
# If the application has included the generic FreeRTOS support, then add in
# the device-specific pieces of the code.
#
ifneq ($(FREERTOS_DIR),)
    FREERTOS_PORTDIR	:= $(PIOS_DEVLIB)libraries/FreeRTOS/Source
    SRC			+= $(sort $(wildcard $(FREERTOS_PORTDIR)/portable/GCC/ARM_CM4F/*.c))
    EXTRAINCDIRS	+= $(FREERTOS_PORTDIR)/portable/GCC/ARM_CM4F
    include $(PIOSCOMMON)/libraries/msheap/library.mk
endif
