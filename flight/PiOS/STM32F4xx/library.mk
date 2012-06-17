#
# Rules to (help) build the F4xx device support.
#

#
# Directory containing this makefile
#
PIOS_DEVLIB			:=	$(dir $(lastword $(MAKEFILE_LIST)))

#
# Hardcoded linker script names for now
#
LINKER_SCRIPTS_APP	 =	$(PIOS_DEVLIB)/link_STM32F4xx_OP_memory.ld \
						$(PIOS_DEVLIB)/link_STM32F4xx_sections.ld

LINKER_SCRIPTS_BL	 =	$(PIOS_DEVLIB)/link_STM32F4xx_BL_memory.ld \
						$(PIOS_DEVLIB)/link_STM32F4xx_sections.ld

#
# Compiler options implied by the F4xx
#
CDEFS				+= -DSTM32F4XX
CDEFS				+= -DHSE_VALUE=$(OSCILLATOR_FREQ)
CDEFS 				+= -DUSE_STDPERIPH_DRIVER
ARCHFLAGS			+= -mcpu=cortex-m4 -march=armv7e-m -mfpu=fpv4-sp-d16 -mfloat-abi=hard

#
# PIOS device library source and includes
#
SRC					+=	$(wildcard $(PIOS_DEVLIB)*.c)
EXTRAINCDIRS		+=	$(PIOS_DEVLIB)/inc

#
# CMSIS for the F4
#
include $(PIOSCOMMONLIB)/CMSIS2/library.mk
CMSIS2_DEVICEDIR	:=	$(PIOS_DEVLIB)/Libraries/CMSIS2/Device/ST/STM32F4xx
SRC					+=	$(wildcard $(CMSIS2_DEVICEDIR)/Source/*.c)
EXTRAINCDIRS		+=	$(CMSIS2_DEVICEDIR)/Include

#
# ST Peripheral library
#
PERIPHLIB			 =	$(PIOS_DEVLIB)/Libraries/STM32F4xx_StdPeriph_Driver
SRC					+=	$(wildcard $(PERIPHLIB)/src/*.c)
EXTRAINCDIRS		+=	$(PERIPHLIB)/inc

#
# ST USB OTG library
#
USBOTGLIB			=	$(PIOS_DEVLIB)/Libraries/STM32_USB_OTG_Driver
USBOTGLIB_SRC			=	usb_core.c usb_dcd.c usb_dcd_int.c
SRC				+=	$(addprefix $(USBOTGLIB)/src/,$(USBOTGLIB_SRC))
EXTRAINCDIRS			+=	$(USBOTGLIB)/inc

#
# ST USB Device library
#
USBDEVLIB			=	$(PIOS_DEVLIB)/Libraries/STM32_USB_Device_Library
SRC				+=	$(wildcard $(USBDEVLIB)/Core/src/*.c)
EXTRAINCDIRS			+=	$(USBDEVLIB)/Core/inc

#
# FreeRTOS
#
# If the application has included the generic FreeRTOS support, then add in
# the device-specific pieces of the code.
#
ifneq ($(FREERTOS_DIR),)
FREERTOS_PORTDIR	:=	$(PIOS_DEVLIB)/Libraries/FreeRTOS/Source
SRC					+=	$(wildcard $(FREERTOS_PORTDIR)/portable/GCC/ARM_CM4/*.c)
EXTRAINCDIRS		+=	$(FREERTOS_PORTDIR)/portable/GCC/ARM_CM4
endif

