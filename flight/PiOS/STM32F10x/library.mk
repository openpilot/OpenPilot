#
# Rules to (help) build the F10x device support.
#

#
# Compiler options implied by the F1xx
#
CDEFS				+= -DARM_MATH_CM3

#
# CMSIS for the F1
#
include $(PIOSCOMMONLIB)/CMSIS2/library.mk
