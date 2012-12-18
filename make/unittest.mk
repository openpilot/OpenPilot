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
CXXFLAGS += -g -Wall -Wextra -std=gnu++0x

# All Google Test headers.
GTEST_HEADERS = $(GTEST_DIR)/include/gtest/*.h \
                $(GTEST_DIR)/include/gtest/internal/*.h

# Google Test libraries
GTEST_LIBS = $(GTEST_DIR)/lib/.libs/libgtest_main.a $(GTEST_DIR)/lib/.libs/libgtest.a

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
ALLCPPSRC       := $(wildcard ./*.cpp)
ALLSRCBASE      := $(notdir $(basename $(ALLSRC) $(ALLCPPSRC)))
ALLOBJ          := $(addprefix $(OUTDIR)/, $(addsuffix .o, $(ALLSRCBASE)))

.PHONY: elf
elf: $(OUTDIR)/$(TARGET).elf

$(foreach src,$(ALLSRC),$(eval $(call COMPILE_C_TEMPLATE,$(src))))
$(foreach src,$(ALLCPPSRC),$(eval $(call COMPILE_CXX_TEMPLATE,$(src))))

$(eval $(call LINK_CXX_TEMPLATE,$(OUTDIR)/$(TARGET).elf,$(ALLOBJ) $(GTEST_LIBS)))

.PHONY: run
run: $(OUTDIR)/$(TARGET).elf
	$(V0) @echo " TAP RUN   $(MSG_EXTRA)  $(call toprel, $<)"
	$(V1) $<
