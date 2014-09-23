#
# Rules to add yaffs2 to the PiOS target
#
#
# Note that the PIOS target-specific makefile will detect that YAFFS2_DIR
# has been defined and add in the target-specific pieces separately.
#


#
# Directory containing this makefile
#
YAFFS2_DIR			:=	$(dir $(lastword $(MAKEFILE_LIST)))

# Compiler options 
#
CDEFS				+= -DCONFIG_YAFFS_DIRECT
CDEFS				+= -DCONFIG_YAFFS_DEFINES_TYPES
CDEFS				+= -DCONFIG_YAFFS_PROVIDE_DEFS
CDEFS				+= -DCONFIG_YAFFSFS_PROVIDE_VALUES
#ARCHFLAGS			+= -DARCH_POSIX

#
# Yaffs2 device library source and includes
#
SRC				+=	$(sort $(wildcard $(YAFFS2_DIR)*.c))
EXTRAINCDIRS			+=	$(YAFFS2_DIR)/inc
