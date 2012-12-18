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

# Flags passed to the preprocessor.
CPPFLAGS += -I$(GTEST_DIR)/include

# Flags passed to the C++ compiler.
CXXFLAGS += -g -Wall -Wextra

# Google Test needs the pthread library
LDFLAGS += -lpthread

#################################
#
# Template to build the user test
#
#################################

# Need to disable THUMB mode for unit tests
override THUMB :=

EXTRAINCDIRS    += .
ALLSRC          := $(SRC) $(wildcard ./*.c)
ALLCPPSRC       := $(wildcard ./*.cpp) $(GTEST_DIR)/src/gtest_main.cc
ALLSRCBASE      := $(notdir $(basename $(ALLSRC) $(ALLCPPSRC)))
ALLOBJ          := $(addprefix $(OUTDIR)/, $(addsuffix .o, $(ALLSRCBASE)))

$(foreach src,$(ALLSRC),$(eval $(call COMPILE_C_TEMPLATE,$(src))))
$(foreach src,$(ALLCPPSRC),$(eval $(call COMPILE_CXX_TEMPLATE,$(src))))


# Specific extensions to CPPFLAGS only for the google test library
$(OUTDIR)/gtest-all.o : CPPFLAGS += -I$(GTEST_DIR)
$(eval $(call COMPILE_CXX_TEMPLATE, $(GTEST_DIR)/src/gtest-all.cc))

$(eval $(call LINK_CXX_TEMPLATE,$(OUTDIR)/$(TARGET).elf,$(ALLOBJ) $(OUTDIR)/gtest-all.o))

.PHONY: elf
elf: $(OUTDIR)/$(TARGET).elf

.PHONY: xml
xml: $(OUTDIR)/$(TARGET).xml

$(OUTDIR)/$(TARGET).xml: $(OUTDIR)/$(TARGET).elf
	$(V0) @echo " TEST XML  $(MSG_EXTRA)  $(call toprel, $@)"
	$(V1) $< --gtest_output=xml:$(OUTDIR)/$(TARGET).xml > /dev/null

.PHONY: run
run: $(OUTDIR)/$(TARGET).elf
	$(V0) @echo " TEST RUN  $(MSG_EXTRA)  $(call toprel, $<)"
	$(V1) $<
