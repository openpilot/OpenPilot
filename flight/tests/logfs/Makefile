###############################################################################
# @file       Makefile
# @author     PhoenixPilot, http://github.com/PhoenixPilot, Copyright (C) 2012
#             Copyright (c) 2013, The OpenPilot Team, http://www.openpilot.org
# @addtogroup 
# @{
# @addtogroup 
# @{
# @brief Makefile for unit test
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

ifndef OPENPILOT_IS_COOL
    $(error Top level Makefile must be used to build this target)
endif

include $(ROOT_DIR)/make/firmware-defs.mk

EXTRAINCDIRS += $(TOPDIR)
EXTRAINCDIRS += $(PIOS)/inc

SRC += $(PIOS)/common/pios_flashfs_logfs.c

CFLAGS += "-DFLASH_IMAGE_FILE=\"$(OUTDIR)/theflash.bin\""

include $(ROOT_DIR)/make/unittest.mk
