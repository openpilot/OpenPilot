#
# Rules to add CMSIS2 to a PiOS target
#

CMSIS2_DIR		:=	$(dir $(lastword $(MAKEFILE_LIST)))
EXTRAINCDIRS		+=	$(CMSIS2_DIR)/Include
