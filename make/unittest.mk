###############################################################################
# @file       unittest.mk
# @author     PhoenixPilot, http://github.com/PhoenixPilot, Copyright (C) 2012
#             Copyright (c) 2013, The OpenPilot Team, http://www.openpilot.org
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

# Use native toolchain and disable THUMB mode for unit tests
override ARM_SDK_PREFIX :=
override THUMB :=

# Unit test source files
ALLSRC     := $(SRC) $(wildcard ./*.c)
ALLCPPSRC  := $(wildcard ./*.cpp) $(GTEST_DIR)/src/gtest_main.cc
ALLSRCBASE := $(notdir $(basename $(ALLSRC) $(ALLCPPSRC)))
ALLOBJ     := $(addprefix $(OUTDIR)/, $(addsuffix .o, $(ALLSRCBASE)))

$(foreach src,$(ALLSRC),$(eval $(call COMPILE_C_TEMPLATE,$(src))))
$(foreach src,$(ALLCPPSRC),$(eval $(call COMPILE_CXX_TEMPLATE,$(src))))

# Specific extensions to CPPFLAGS only for the google test library
$(OUTDIR)/gtest-all.o: CPPFLAGS += -I$(GTEST_DIR)

$(eval $(call COMPILE_CXX_TEMPLATE, $(GTEST_DIR)/src/gtest-all.cc))
$(eval $(call LINK_CXX_TEMPLATE,$(OUTDIR)/$(TARGET).elf,$(ALLOBJ) $(OUTDIR)/gtest-all.o))

# Flags passed to the preprocessor
CPPFLAGS += -I$(GTEST_DIR)/include

# Flags passed to the C++ compiler
CXXFLAGS += -g -Wall -Wextra

# Flags passed to the C compiler
CONLYFLAGS += -std=gnu99

# UNIT_TEST allows to for example to have optional test fixture code enabled or private code exposed in application modules. 
CFLAGS += -DUNIT_TEST
CPPFLAGS += -DUNIT_TEST

# Common compiler flags
CFLAGS += -O0 -g
CFLAGS += -Wall -Werror
CFLAGS += $(patsubst %,-I%,$(EXTRAINCDIRS))

# Google Test needs the pthread library
LDFLAGS += -lpthread

.PHONY: elf
elf: $(OUTDIR)/$(TARGET).elf

.PHONY: xml
xml: $(OUTDIR)/test-reports/$(TARGET).xml

$(OUTDIR)/test-reports/$(TARGET).xml: $(OUTDIR)/$(TARGET).elf
	$(V0) @echo " TEST XML  $(MSG_EXTRA)  $(call toprel, $@)"
	$(V1) $< --gtest_output=xml:$@ > /dev/null

.PHONY: run
run: $(OUTDIR)/$(TARGET).elf
	$(V0) @echo " TEST RUN  $(MSG_EXTRA)  $(call toprel, $<)"
	$(V1) $<
