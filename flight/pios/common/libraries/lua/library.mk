#
# Rules to add lua to the PiOS target
#
#
# Note that the PIOS target-specific makefile will detect that LUA_DIR
# has been defined and add in the target-specific pieces separately.
#


#
# Directory containing this makefile
#
LUA_DIR		:= $(dir $(lastword $(MAKEFILE_LIST)))


#
# Version
#
LUA_VERSION := lua-5.2.4


#
# Compiler options
#
CDEFS += -DCONFIG_BUILD_LUA -DLUA -Wno-unsuffixed-float-constants


#
# lua library source and includes
#
LUA_SRC = lapi.c lcode.c lctype.c ldebug.c ldo.c ldump.c lfunc.c lgc.c llex.c lmem.c \
          lobject.c lopcodes.c lparser.c lstate.c lstring.c ltable.c lrotable.c ltm.c lundump.c lvm.c lzio.c \
          lbaselib.c linit.c lbitlib.c loadlib.c lauxlib.c ltablib.c lcorolib.c
          #lmathlib.c

SRC += $(addprefix $(LUA_DIR)/$(LUA_VERSION)/src/,$(LUA_SRC))

EXTRAINCDIRS += $(LUA_DIR)/$(LUA_VERSION)/src

