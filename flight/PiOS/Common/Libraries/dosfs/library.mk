#
# Rules to add DOSFS to a PiOS target
#

DOSFS_DIR		:=	$(dir $(lastword $(MAKEFILE_LIST)))
SRC			+=	$(wildcard $(DOSFS_DIR)/*.c)
EXTRAINCDIRS		+=	$(DOSFS_DIR)

