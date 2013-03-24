#
# Rules to add FreeRTOS to a PiOS target
#
# Note that the PIOS target-specific makefile will detect that FREERTOS_DIR
# has been defined and add in the target-specific pieces separately.
#

FREERTOS_DIR		:=	$(dir $(lastword $(MAKEFILE_LIST)))/Source
SRC			+=	$(wildcard $(FREERTOS_DIR)/*.c)
EXTRAINCDIRS		+=	$(FREERTOS_DIR)/include

