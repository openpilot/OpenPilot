#
# Copyright (c) 2010-2013, The OpenPilot Team, http://www.openpilot.org
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
#

ifndef OPENPILOT_IS_COOL
    $(error Top level Makefile must be used to build this target)
endif

# Define toolchain component names.
CC      = $(ARM_SDK_PREFIX)gcc
CXX     = $(ARM_SDK_PREFIX)g++
AR      = $(ARM_SDK_PREFIX)ar
OBJCOPY = $(ARM_SDK_PREFIX)objcopy
OBJDUMP = $(ARM_SDK_PREFIX)objdump
SIZE    = $(ARM_SDK_PREFIX)size
NM      = $(ARM_SDK_PREFIX)nm
STRIP   = $(ARM_SDK_PREFIX)strip

THUMB   = -mthumb

# Add a board designator to the terse message text
ifeq ($(ENABLE_MSG_EXTRA),yes)
    MSG_EXTRA := [$(BUILD_TYPE)|$(BOARD_SHORT_NAME)]
else
    MSG_EXTRA :=
endif

# Define Messages
MSG_FORMATERROR      = $(QUOTE) Can not handle output-format$(QUOTE)
MSG_MODINIT          = $(QUOTE) MODINIT   $(MSG_EXTRA) $(QUOTE)
MSG_SIZE             = $(QUOTE) SIZE      $(MSG_EXTRA) $(QUOTE)
MSG_LOAD_FILE        = $(QUOTE) BIN/HEX   $(MSG_EXTRA) $(QUOTE)
MSG_BIN_OBJ          = $(QUOTE) BINO      $(MSG_EXTRA) $(QUOTE)
MSG_STRIP_FILE       = $(QUOTE) STRIP     $(MSG_EXTRA) $(QUOTE)
MSG_EXTENDED_LISTING = $(QUOTE) LIS       $(MSG_EXTRA) $(QUOTE)
MSG_SYMBOL_TABLE     = $(QUOTE) NM        $(MSG_EXTRA) $(QUOTE)
MSG_ARCHIVING        = $(QUOTE) AR        $(MSG_EXTRA) $(QUOTE)
MSG_LINKING          = $(QUOTE) LD        $(MSG_EXTRA) $(QUOTE)
MSG_COMPILING        = $(QUOTE) CC        $(MSG_EXTRA) $(QUOTE)
MSG_COMPILING_ARM    = $(QUOTE) CC-ARM    $(MSG_EXTRA) $(QUOTE)
MSG_COMPILINGCXX     = $(QUOTE) CXX       $(MSG_EXTRA) $(QUOTE)
MSG_COMPILINGCXX_ARM = $(QUOTE) CXX-ARM   $(MSG_EXTRA) $(QUOTE)
MSG_ASSEMBLING       = $(QUOTE) AS        $(MSG_EXTRA) $(QUOTE)
MSG_ASSEMBLING_ARM   = $(QUOTE) AS-ARM    $(MSG_EXTRA) $(QUOTE)
MSG_CLEANING         = $(QUOTE) CLEAN     $(MSG_EXTRA) $(QUOTE)
MSG_ASMFROMC         = $(QUOTE) AS(C)     $(MSG_EXTRA) $(QUOTE)
MSG_ASMFROMC_ARM     = $(QUOTE) AS(C)-ARM $(MSG_EXTRA) $(QUOTE)
MSG_PYMITEINIT       = $(QUOTE) PY        $(MSG_EXTRA) $(QUOTE)
MSG_OPFIRMWARE       = $(QUOTE) OPFW      $(MSG_EXTRA) $(QUOTE)
MSG_FWINFO           = $(QUOTE) FWINFO    $(MSG_EXTRA) $(QUOTE)
MSG_JTAG_PROGRAM     = $(QUOTE) JTAG-PGM  $(MSG_EXTRA) $(QUOTE)
MSG_JTAG_WIPE        = $(QUOTE) JTAG-WIPE $(MSG_EXTRA) $(QUOTE)
MSG_PADDING          = $(QUOTE) PADDING   $(MSG_EXTRA) $(QUOTE)
MSG_FLASH_IMG        = $(QUOTE) FLASH_IMG $(MSG_EXTRA) $(QUOTE)

# Function for converting an absolute path to one relative
# to the top of the source tree.
toprel = $(subst $(realpath $(ROOT_DIR))/,,$(abspath $(1)))

# Display compiler version information.
.PHONY: gccversion
gccversion:
	@$(CC) --version

# Create final output file (.hex) from ELF output file.
%.hex: %.elf
	@$(ECHO) $(MSG_LOAD_FILE) $(call toprel, $@)
	$(V1) $(OBJCOPY) -O ihex $< $@

# Create stripped output file (.elf.stripped) from ELF output file.
%.elf.stripped: %.elf
	@$(ECHO) $(MSG_STRIP_FILE) $(call toprel, $@)
	$(V1) $(STRIP) --strip-unneeded $< -o $@

# Create final output file (.bin) from ELF output file.
%.bin: %.elf
	@$(ECHO) $(MSG_LOAD_FILE) $(call toprel, $@)
	$(V1) $(OBJCOPY) -O binary $< $@

%.bin: %.o
	@$(ECHO) $(MSG_LOAD_FILE) $(call toprel, $@)
	$(V1) $(OBJCOPY) -O binary $< $@

%.bin.o: %.bin
	@$(ECHO) $(MSG_BIN_OBJ) $(call toprel, $@)
	$(V1) $(OBJCOPY) -I binary -O elf32-littlearm --binary-architecture arm \
		--rename-section .data=.rodata,alloc,load,readonly,data,contents \
		--wildcard \
		--redefine-sym _binary_$(subst :,_,$(subst -,_,$(subst .,_,$(subst /,_,$<))))_start=_binary_start \
		--redefine-sym _binary_$(subst :,_,$(subst -,_,$(subst .,_,$(subst /,_,$<))))_end=_binary_end \
		--redefine-sym _binary_$(subst :,_,$(subst -,_,$(subst .,_,$(subst /,_,$<))))_size=_binary_size \
		$< $@

# Create extended listing file/disassambly from ELF output file.
# using objdump testing: option -C
%.lss: %.elf
	@$(ECHO) $(MSG_EXTENDED_LISTING) $(call toprel, $@)
	$(V1) $(OBJDUMP) -h -S -C -r $< > $@

# Create a symbol table from ELF output file.
%.sym: %.elf
	@$(ECHO) $(MSG_SYMBOL_TABLE) $(call toprel, $@)
	$(V1) $(NM) -n $< > $@

define SIZE_TEMPLATE
.PHONY: size
size: $(1)_size

.PHONY: $(1)_size
$(1)_size: $(1)
	@$(ECHO) $(MSG_SIZE) $$(call toprel, $$<)
	$(V1) $(SIZE) -A $$<
endef

# OpenPilot firmware image template
#  $(1) = path to bin file
#  $(2) = boardtype in hex
#  $(3) = board revision in hex
define OPFW_TEMPLATE
FORCE:

$(1).firmwareinfo.c: $(1) $(ROOT_DIR)/make/templates/firmwareinfotemplate.c FORCE
	@$(ECHO) $(MSG_FWINFO) $$(call toprel, $$@)
	$(V1) $(VERSION_INFO) \
		--template=$(ROOT_DIR)/make/templates/firmwareinfotemplate.c \
		--outfile=$$@ \
		--image=$(1) \
		--type=$(2) \
		--revision=$(3) \
		--uavodir=$(ROOT_DIR)/shared/uavobjectdefinition

$(eval $(call COMPILE_C_TEMPLATE, $(1).firmwareinfo.c))

$(OUTDIR)/$(notdir $(basename $(1))).opfw : $(1) $(1).firmwareinfo.bin
	@$(ECHO) $(MSG_OPFIRMWARE) $$(call toprel, $$@)
	$(V1) $(CAT) $(1) $(1).firmwareinfo.bin > $$@
endef

# Assemble: create object files from assembler source files.
define ASSEMBLE_TEMPLATE
$(OUTDIR)/$(notdir $(basename $(1))).o : $(1)
	@$(ECHO) $(MSG_ASSEMBLING) $$(call toprel, $$<)
	$(V1) $(CC) -c $(THUMB) $$(ASFLAGS) $$< -o $$@
endef

# Assemble: create object files from assembler source files. ARM-only
define ASSEMBLE_ARM_TEMPLATE
$(OUTDIR)/$(notdir $(basename $(1))).o : $(1)
	@$(ECHO) $(MSG_ASSEMBLING_ARM) $$(call toprel, $$<)
	$(V1) $(CC) -c $$(ASFLAGS) $$< -o $$@
endef

# Compile: create object files from C source files.
define COMPILE_C_TEMPLATE
$(OUTDIR)/$(notdir $(basename $(1))).o : $(1)
	@$(ECHO) $(MSG_COMPILING) $$(call toprel, $$<)
	$(V1) $(CC) -c $(THUMB) $$(CFLAGS) $$(CONLYFLAGS) $$< -o $$@
endef

# Compile: create object files from C source files. ARM-only
define COMPILE_C_ARM_TEMPLATE
$(OUTDIR)/$(notdir $(basename $(1))).o : $(1)
	@$(ECHO) $(MSG_COMPILING_ARM) $$(call toprel, $$<)
	$(V1) $(CC) -c $$(CFLAGS) $$(CONLYFLAGS) $$< -o $$@
endef

# Compile: create object files from C++ source files.
define COMPILE_CXX_TEMPLATE
$(OUTDIR)/$(notdir $(basename $(1))).o : $(1)
	@$(ECHO) $(MSG_COMPILINGCXX) $$(call toprel, $$<)
	$(V1) $(CXX) -c $(THUMB) $$(CFLAGS) $$(CPPFLAGS) $$(CXXFLAGS) $$< -o $$@
endef

# Compile: create object files from C++ source files. ARM-only
define COMPILE_CXX_ARM_TEMPLATE
$(OUTDIR)/$(notdir $(basename $(1))).o : $(1)
	@$(ECHO) $(MSG_COMPILINGCXX_ARM) $$(call toprel, $$<)
	$(V1) $(CPP) -c $$(CFLAGS) $$(CPPFLAGS) $$(CXXFLAGS) $$< -o $$@
endef

# Archive: create ar library file from object files.
#   $1 = library file to produce
#   $2 = list of object files that make up the library file
#   $3 = optional object files directory
define ARCHIVE_TEMPLATE
.SECONDARY : $(1)
.PRECIOUS : $(2)
$(1):  $(2)
	@$(ECHO) $(MSG_ARCHIVING) $$(call toprel, $$@)
ifeq ($(3),)
	$(V1) $(AR) rcs $$@ $(2)
else
#	This is a workaround for Windows CreateProcess() line length
#	limitation. It is assumed that if the object files directory
#	is given, all object files are in that directory.
	$(V1) ( \
		pwd=`pwd` && \
		cd $(3) && \
		$(AR) rcs $$@ $(notdir $(2)) && \
		cd $$$${pwd} >/dev/null \
	      )
endif
endef

# Link: create ELF output file from object files.
#   $1 = elf file to produce
#   $2 = list of object files that make up the elf file
#   $3 = optional list of libraries to build and link
define LINK_TEMPLATE
.SECONDARY : $(1)
.PRECIOUS : $(2) $(3)
$(1):  $(2) $(3)
	@$(ECHO) $(MSG_LINKING) $$(call toprel, $$@)
	$(V1) $(CC) $(THUMB) $$(CFLAGS) $(2) $(3) --output $$@ $$(LDFLAGS)
endef

# Link: create ELF output file from object files.
#   $1 = elf file to produce
#   $2 = list of object files that make up the elf file
define LINK_CXX_TEMPLATE
.SECONDARY : $(1)
.PRECIOUS : $(2)
$(1):  $(2)
	@$(ECHO) $(MSG_LINKING) $$(call toprel, $$@)
	$(V1) $(CXX) $(THUMB) $$(CFLAGS) $(2) --output $$@ $$(LDFLAGS)
endef

# Compile: create assembler files from C source files. ARM/Thumb
define PARTIAL_COMPILE_TEMPLATE
$($(1):.c=.s) : %.s : %.c
	@$(ECHO) $(MSG_ASMFROMC) $$(call toprel, $$<)
	$(V1) $(CC) $(THUMB) -S $$(CFLAGS) $$(CONLYFLAGS) $$< -o $$@
endef

# Compile: create assembler files from C source files. ARM only
define PARTIAL_COMPILE_ARM_TEMPLATE
$($(1):.c=.s) : %.s : %.c
	@$(ECHO) $(MSG_ASMFROMC_ARM) $$(call toprel, $$<)
	$(V1) $(CC) -S $$(CFLAGS) $$(CONLYFLAGS) $$< -o $$@
endef

# $(1) = Name of binary image to write
# $(2) = Base of flash region to write/wipe
# $(3) = Size of flash region to write/wipe
# $(4) = OpenOCD JTAG interface configuration file to use
# $(5) = OpenOCD configuration file to use
define JTAG_TEMPLATE
# ---------------------------------------------------------------------------
# Options for OpenOCD flash-programming
# see openocd.pdf/openocd.texi for further information

# debug level
OOCD_JTAG_SETUP  = -d0
# interface and board/target settings (using the OOCD target-library here)
OOCD_JTAG_SETUP += -s $(ROOT_DIR)/flight/Project/OpenOCD
OOCD_JTAG_SETUP += -f $(4) -f $(5)

# initialize
OOCD_BOARD_RESET = -c init
# show the targets
#OOCD_BOARD_RESET += -c targets
# commands to prepare flash-write
OOCD_BOARD_RESET += -c "reset halt"

.PHONY: program
program: $(1)
	@$(ECHO) $(MSG_JTAG_PROGRAM) $$(call toprel, $$<)
	$(V1) $(OPENOCD) \
		$$(OOCD_JTAG_SETUP) \
		$$(OOCD_BOARD_RESET) \
		-c "flash write_image erase $$< $(2) bin" \
		-c "verify_image $$< $(2) bin" \
		-c "reset run" \
		-c "shutdown"

.PHONY: wipe
wipe:
	@$(ECHO) $(MSG_JTAG_WIPE) wiping $(3) bytes starting from $(2)
	$(V1) $(OPENOCD) \
		$$(OOCD_JTAG_SETUP) \
		$$(OOCD_BOARD_RESET) \
		-c "flash erase_address pad $(2) $(3)" \
		-c "reset run" \
		-c "shutdown"
endef
