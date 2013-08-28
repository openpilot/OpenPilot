#
# Rules to (help) build the F30x device support.
#

#
# Directory containing this makefile
#
PIOS_DEVLIB			:=	$(dir $(lastword $(MAKEFILE_LIST)))

#
# Hardcoded linker script names for now
#
LINKER_SCRIPTS_APP	 =	$(PIOS_DEVLIB)/link_STM32F30x_FW_memory.ld \
						$(PIOS_DEVLIB)/link_STM32F30x_sections.ld

LINKER_SCRIPTS_BL	 =	$(PIOS_DEVLIB)/link_STM32F30x_BL_memory.ld \
						$(PIOS_DEVLIB)/link_STM32F30x_sections.ld

#
# Compiler options implied by the F30x
#
CDEFS				+= -DSTM32F30X
CDEFS			+= -DSYSCLK_FREQ=$(SYSCLK_FREQ)
CDEFS				+= -DHSE_VALUE=$(OSCILLATOR_FREQ)
CDEFS 				+= -DUSE_STDPERIPH_DRIVER
ARCHFLAGS			+= -mcpu=cortex-m4 -march=armv7e-m -mfpu=fpv4-sp-d16 -mfloat-abi=hard

#
# PIOS device library source and includes
#
SRC					+=	$(wildcard $(PIOS_DEVLIB)*.c)
EXTRAINCDIRS		+=	$(PIOS_DEVLIB)/inc

#
# CMSIS for the F3
#
include $(PIOSCOMMON)/libraries/CMSIS3/library.mk
CMSIS3_DEVICEDIR	:=	$(PIOS_DEVLIB)/Libraries/CMSIS3/Device/ST/STM32F30x
SRC					+=	$(wildcard $(CMSIS3_DEVICEDIR)/Source/$(BOARD_NAME)/*.c)
EXTRAINCDIRS		+=	$(CMSIS3_DEVICEDIR)/Include

#
# ST Peripheral library
#
PERIPHLIB			 =	$(PIOS_DEVLIB)/Libraries/STM32F30x_StdPeriph_Driver
SRC					+=	$(wildcard $(PERIPHLIB)/src/*.c)
EXTRAINCDIRS		+=	$(PERIPHLIB)/inc

#
# ST USB FS library
#
USBFSLIB			=	$(PIOS_DEVLIB)/Libraries/STM32_USB-FS-Device_Driver
USBFSLIB_SRC		=	usb_core.c usb_init.c usb_int.c usb_mem.c usb_regs.c usb_sil.c
SRC					+=	$(addprefix $(USBFSLIB)/src/,$(USBFSLIB_SRC))
EXTRAINCDIRS		+=	$(USBFSLIB)/inc

#
# FreeRTOS
#
# If the application has included the generic FreeRTOS support, then add in
# the device-specific pieces of the code.
#
ifneq ($(FREERTOS_DIR),)
FREERTOS_PORTDIR	:=	$(PIOS_DEVLIB)/Libraries/FreeRTOS/Source
SRC					+=	$(wildcard $(FREERTOS_PORTDIR)/portable/GCC/ARM_CM4F/*.c)
EXTRAINCDIRS		+=	$(FREERTOS_PORTDIR)/portable/GCC/ARM_CM4F
include $(PIOSCOMMON)/libraries/msheap/library.mk
endif

