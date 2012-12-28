#
# Rules to (help) build the F10x device support.
#

#
# CMSIS for the F1
#
CDEFS				+= -DARM_MATH_CM3
include $(PIOSCOMMONLIB)/CMSIS2/library.mk
