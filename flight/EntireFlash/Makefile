#####
# Makefile for Entire Flash (EF) images
#
# The OpenPilot Team, http://www.openpilot.org, Copyright (C) 2012.
#
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
#####

WHEREAMI := $(dir $(lastword $(MAKEFILE_LIST)))
TOP      := $(realpath $(WHEREAMI)/../../)
include $(TOP)/make/firmware-defs.mk
include $(TOP)/make/boards/$(BOARD_NAME)/board-info.mk

# Target file name (without extension).
TARGET := ef_$(BOARD_NAME)

# Directory for output files (lst, obj, dep, elf, sym, map, hex, bin etc.)
OUTDIR := $(TOP)/build/$(TARGET)

.PHONY: bin
bin: $(OUTDIR)/$(TARGET).bin

$(OUTDIR)/$(TARGET).bin: BL_BIN := $(TOP)/build/bl_$(BOARD_NAME)/bl_$(BOARD_NAME).bin
$(OUTDIR)/$(TARGET).bin: FW_BIN := $(TOP)/build/fw_$(BOARD_NAME)/fw_$(BOARD_NAME).bin
$(OUTDIR)/$(TARGET).bin: $(BL_BIN) $(FW_BIN)
	$(V0) @echo " FLASH IMG  $@"
	$(V1) cat $(BL_BIN) $(FW_BIN) > $@

.PHONY: dfu
dfu: $(OUTDIR)/$(TARGET).bin
	$(V0) @echo " DFU RESCUE $<"
	$(V1) ( \
		sudo $(DFU_CMD) -l && \
		sudo $(DFU_CMD) -d 0483:df11 -c 1 -i 0 -a 0 -D $< -s $(BL_BANK_BASE) ; \
	      )

