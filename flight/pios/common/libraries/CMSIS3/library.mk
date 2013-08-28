#
# Rules to add CMSIS3 to a PiOS target
#

CMSIS3_DIR		:=	$(dir $(lastword $(MAKEFILE_LIST)))
EXTRAINCDIRS		+=	$(CMSIS3_DIR)/Include
