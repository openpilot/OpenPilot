#
# Rules to add CMSIS2 to a PiOS target
#

CMSIS2_DIR		:=	$(dir $(lastword $(MAKEFILE_LIST)))
CMSIS_DSP_LIB		:=	$(CMSIS2_DIR)/DSP_Lib/Source
EXTRAINCDIRS		+=	$(CMSIS2_DIR)/Include
ifeq ($(USE_DSP_LIB),1)
    SRC			+=  $(wildcard $(CMSIS_DSP_LIB)/*/*.c)
endif