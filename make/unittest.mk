###############################################################################
# @file       unittest.mk
# @author     PhoenixPilot, http://github.com/PhoenixPilot, Copyright (C) 2012
# @addtogroup 
# @{
# @addtogroup 
# @{
# @brief Makefile template for unit tests
###############################################################################
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

# Need to disable THUMB mode for unit tests
override THUMB :=

EXTRAINCDIRS    += .
ALLSRC          := $(SRC) $(wildcard ./*.c)
ALLSRCBASE      := $(notdir $(basename $(ALLSRC)))
ALLOBJ          := $(addprefix $(OUTDIR)/, $(addsuffix .o, $(ALLSRCBASE)))

.PHONY: $(TARGET)
$(TARGET): | $(OUTDIR)
$(TARGET): $(OUTDIR)/$(TARGET).elf

$(OUTDIR):
	$(V1) mkdir -p $@

$(foreach src,$(ALLSRC),$(eval $(call COMPILE_C_TEMPLATE,$(src))))
$(eval $(call LINK_TEMPLATE,$(OUTDIR)/$(TARGET).elf,$(ALLOBJ)))

