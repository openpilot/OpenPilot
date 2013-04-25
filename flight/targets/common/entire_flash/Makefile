#
# Copyright (c) 2009-2013, The OpenPilot Team, http://www.openpilot.org
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

include ../../boards/$(BOARD_NAME)/board-info.mk
include $(ROOT_DIR)/make/firmware-defs.mk

# Paths
TOPDIR		= .
ENTIRE_FLASH	= $(PYTHON) "$(TOPDIR)/entire-flash.py"

ifeq ($(V), 1)
    EF_VERBOSE := --verbose
else
    EF_VERBOSE :=
endif

.PHONY: bin
bin: $(OUTDIR)/$(TARGET).bin

BL_BIN = $(BUILD_DIR)/bl_$(BOARD_NAME)/bl_$(BOARD_NAME).bin
FW_BIN = $(BUILD_DIR)/fw_$(BOARD_NAME)/fw_$(BOARD_NAME).bin
FWINFO_BIN = $(FW_BIN).firmwareinfo.bin

$(OUTDIR)/$(TARGET).bin: $(BL_BIN) $(FW_BIN)
	$(V0) @$(ECHO) $(MSG_FLASH_IMG) $@
	$(V1) $(ENTIRE_FLASH) $(EF_VERBOSE) \
		--bl-bank-base=$(BL_BANK_BASE) \
		--bl-bank-size=$(BL_BANK_SIZE) \
		--fw-bank-base=$(FW_BANK_BASE) \
		--fw-bank-size=$(FW_BANK_SIZE) \
		--fw-desc-size=$(FW_DESC_SIZE) \
		--bl-bin-path="$(BL_BIN)" \
		--fw-bin-path="$(FW_BIN)" \
		--fwinfo-bin-path="$(FWINFO_BIN)" \
		--outfile="$@"

.PHONY: dfu
dfu: $(OUTDIR)/$(TARGET).bin
	$(V0) @echo " DFU RESCUE $<"
	$(V1) ( \
		sudo $(DFU_CMD) -l && \
		sudo $(DFU_CMD) -d 0483:df11 -c 1 -i 0 -a 0 -D $< -s $(BL_BANK_BASE) ; \
	      )
