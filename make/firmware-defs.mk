# Toolchain prefix (i.e arm-elf- -> arm-elf-gcc.exe)
TCHAIN_PREFIX ?= arm-none-eabi-

# Define toolchain component names.
CC      = $(TCHAIN_PREFIX)gcc
CPP     = $(TCHAIN_PREFIX)g++
AR      = $(TCHAIN_PREFIX)ar
OBJCOPY = $(TCHAIN_PREFIX)objcopy
OBJDUMP = $(TCHAIN_PREFIX)objdump
SIZE    = $(TCHAIN_PREFIX)size
NM      = $(TCHAIN_PREFIX)nm
STRIP   = $(TCHAIN_PREFIX)strip

THUMB   = -mthumb

# Test if quotes are needed for the echo-command
result = ${shell echo "test"}
ifeq (${result}, test)
	quote = '
# This line is just to clear out the single quote above '
else
	quote =
endif

# Define Messages
# English
MSG_FORMATERROR      := ${quote} Can not handle output-format${quote}
MSG_MODINIT          := ${quote} MODINIT   ${quote}
MSG_SIZE             := ${quote} SIZE      ${quote}
MSG_LOAD_FILE        := ${quote} BIN/HEX   ${quote}
MSG_BIN_OBJ          := ${quote} BINO      ${quote}
MSG_STRIP_FILE       := ${quote} STRIP     ${quote}
MSG_EXTENDED_LISTING := ${quote} LIS       ${quote}
MSG_SYMBOL_TABLE     := ${quote} NM        ${quote}
MSG_LINKING          := ${quote} LD        ${quote}
MSG_COMPILING        := ${quote} CC        ${quote}
MSG_COMPILING_ARM    := ${quote} CC-ARM    ${quote}
MSG_COMPILINGCPP     := ${quote} CXX       ${quote}
MSG_COMPILINGCPP_ARM := ${quote} CXX-ARM   ${quote}
MSG_ASSEMBLING       := ${quote} AS        ${quote}
MSG_ASSEMBLING_ARM   := ${quote} AS-ARM    ${quote}
MSG_CLEANING         := ${quote} CLEAN     ${quote}
MSG_ASMFROMC         := ${quote} AS(C)     ${quote}
MSG_ASMFROMC_ARM     := ${quote} AS(C)-ARM ${quote}
MSG_PYMITEINIT       := ${quote} PY        ${quote}

toprel = $(subst $(realpath $(TOP))/,,$(abspath $(1)))

# Display compiler version information.
.PHONY: gccversion
gccversion :
	@$(CC) --version

# Create final output file (.hex) from ELF output file.
%.hex: %.elf
	@echo $(MSG_LOAD_FILE) $(call toprel, $@)
	$(V1) $(OBJCOPY) -O ihex $< $@

# Create stripped output file (.elf.stripped) from ELF output file.
%.elf.stripped: %.elf
	@echo $(MSG_STRIP_FILE) $(call toprel, $@)
	$(V1) $(STRIP) --strip-unneeded $< -o $@

# Create final output file (.bin) from ELF output file.
%.bin: %.elf
	@echo $(MSG_LOAD_FILE) $(call toprel, $@)
	$(V1) $(OBJCOPY) -O binary $< $@

%.bin.o: %.bin
	@echo $(MSG_BIN_OBJ) $(call toprel, $@)
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
	@echo $(MSG_EXTENDED_LISTING) $(call toprel, $@)
	$(V1) $(OBJDUMP) -h -S -C -r $< > $@

# Create a symbol table from ELF output file.
%.sym: %.elf
	@echo $(MSG_SYMBOL_TABLE) $(call toprel, $@)
	$(V1) $(NM) -n $< > $@

define SIZE_TEMPLATE
.PHONY: $(1)_size
$(1)_size: $(1)
	@echo $(MSG_SIZE) $$(call toprel, $$<)
	$(V1) $(SIZE) -A $$<
endef

# Assemble: create object files from assembler source files.
define ASSEMBLE_TEMPLATE
$(OUTDIR)/$(notdir $(basename $(1))).o : $(1)
	@echo $(MSG_ASSEMBLING) $$(call toprel, $$<)
	$(V1) $(CC) -c -mthumb $$(ASFLAGS) $$< -o $$@
endef

# Assemble: create object files from assembler source files. ARM-only
define ASSEMBLE_ARM_TEMPLATE
$(OUTDIR)/$(notdir $(basename $(1))).o : $(1)
	@echo $(MSG_ASSEMBLING_ARM) $$(call toprel, $$<)
	$(V1) $(CC) -c $$(ASFLAGS) $$< -o $$@
endef

# Compile: create object files from C source files.
define COMPILE_C_TEMPLATE
$(OUTDIR)/$(notdir $(basename $(1))).o : $(1)
	@echo $(MSG_COMPILING) $$(call toprel, $$<)
	$(V1) $(CC) -c -mthumb $$(CFLAGS) $$(CONLYFLAGS) $$< -o $$@
endef

# Compile: create object files from C source files. ARM-only
define COMPILE_C_ARM_TEMPLATE
$(OUTDIR)/$(notdir $(basename $(1))).o : $(1)
	@echo $(MSG_COMPILING_ARM) $$(call toprel, $$<)
	$(V1) $(CC) -c $$(CFLAGS) $$(CONLYFLAGS) $$< -o $$@
endef

# Compile: create object files from C++ source files.
define COMPILE_CPP_TEMPLATE
$(OUTDIR)/$(notdir $(basename $(1))).o : $(1)
	@echo $(MSG_COMPILINGCPP) $$(call toprel, $$<)
	$(V1) $(CC) -c -mthumb $$(CFLAGS) $$(CPPFLAGS) $$< -o $$@
endef

# Compile: create object files from C++ source files. ARM-only
define COMPILE_CPP_ARM_TEMPLATE
$(OUTDIR)/$(notdir $(basename $(1))).o : $(1)
	@echo $(MSG_COMPILINGCPP_ARM) $$(call toprel, $$<)
	$(V1) $(CC) -c $$(CFLAGS) $$(CPPFLAGS) $$< -o $$@
endef

# Link: create ELF output file from object files.
#   $1 = elf file to produce
#   $2 = list of object files that make up the elf file
define LINK_TEMPLATE
.SECONDARY : $(1)
.PRECIOUS : $(2)
$(1):  $(2)
	@echo $(MSG_LINKING) $$(call toprel, $$@)
	$(V1) $(CC) -mthumb $$(CFLAGS) $(2) --output $$@ $$(LDFLAGS)
endef

# Compile: create assembler files from C source files. ARM/Thumb
define PARTIAL_COMPILE_TEMPLATE
$($(1):.c=.s) : %.s : %.c
	@echo $(MSG_ASMFROMC) $$(call toprel, $$<)
	$(V1) $(CC) -mthumb -S $$(CFLAGS) $$(CONLYFLAGS) $$< -o $$@
endef

# Compile: create assembler files from C source files. ARM only
define PARTIAL_COMPILE_ARM_TEMPLATE
$($(1):.c=.s) : %.s : %.c
	@echo $(MSG_ASMFROMC_ARM) $$(call toprel, $$<)
	$(V1) $(CC) -S $$(CFLAGS) $$(CONLYFLAGS) $$< -o $$@
endef

# ---------------------------------------------------------------------------
# Options for OpenOCD flash-programming
# see openocd.pdf/openocd.texi for further information

# if OpenOCD is in the $PATH just set OPENOCDEXE=openocd
OOCD_EXE=openocd
# debug level
OOCD_CL=-d0
# interface and board/target settings (using the OOCD target-library here)
OOCD_CL+=-s $(TOP)/flight/Project/OpenOCD
OOCD_CL+=-f foss-jtag.revb.cfg -f stm32.cfg

# initialize
OOCD_CL+=-c init
# show the targets
OOCD_CL+=-c targets
# commands to prepare flash-write
OOCD_CL+= -c "reset halt"
