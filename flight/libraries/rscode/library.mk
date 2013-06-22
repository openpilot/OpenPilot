#
# Rules to add RSCODE to a PiOS target
#

RSCODE_DIR	:=	$(dir $(lastword $(MAKEFILE_LIST)))
RSCODE_SRC	:=	berlekamp.c crcgen.c galois.c rs.c

SRC		+=	$(addprefix $(RSCODE_DIR),$(RSCODE_SRC))
EXTRAINCDIRS	+=	$(RSCODE_DIR)
