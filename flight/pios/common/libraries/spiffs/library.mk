#
# Rules to add spiffs to the PiOS target
#
#
# Note that the PIOS target-specific makefile will detect that SPIFFS_DIR
# has been defined and add in the target-specific pieces separately.
#


#
# Directory containing this makefile
#
SPIFFS_DIR		:=	$(dir $(lastword $(MAKEFILE_LIST)))


# Compiler options
#
CDEFS			+= -DCONFIG_BUILD_SPIFFS


#
# spiffs device library source and includes
#
SRC				+=	$(sort $(wildcard $(SPIFFS_DIR)/src/*.c))
EXTRAINCDIRS	+=	$(SPIFFS_DIR)/src
EXTRAINCDIRS	+=	$(SPIFFS_DIR)/src/default
