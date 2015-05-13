#
# Top level Makefile for the OpenPilot project build system.
# Copyright (c) 2010-2013, The OpenPilot Team, http://www.openpilot.org
# Use 'make help' for instructions.
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

# This top level Makefile passes down some variables to sub-makes through
# the environment. They are explicitly exported using the export keyword.
# Lower level makefiles assume that these variables are defined. To ensure
# that a special magic variable is exported here. It must be checked for
# existance by each sub-make.
export OPENPILOT_IS_COOL := Fuck Yeah!

# It is possible to set OPENPILOT_DL_DIR and/or OPENPILOT_TOOLS_DIR environment
# variables to override local tools download and installation directorys. So the
# same toolchains can be used for all working copies. Particularly useful for CI
# server build agents, but also for local installations.
#
# If no OPENPILOT_* variables found, makefile internal DL_DIR and TOOLS_DIR paths
# will be used. They still can be overriden by the make command line parameters:
# make DL_DIR=/path/to/download/directory TOOLS_DIR=/path/to/tools/directory targets...

# Function for converting Windows style slashes into Unix style
slashfix = $(subst \,/,$(1))

# Function for converting an absolute path to one relative
# to the top of the source tree
toprel = $(subst $(realpath $(ROOT_DIR))/,,$(abspath $(1)))

# Set up some macros for common directories within the tree
export ROOT_DIR    := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))
export DL_DIR      := $(if $(OPENPILOT_DL_DIR),$(call slashfix,$(OPENPILOT_DL_DIR)),$(ROOT_DIR)/downloads)
export TOOLS_DIR   := $(if $(OPENPILOT_TOOLS_DIR),$(call slashfix,$(OPENPILOT_TOOLS_DIR)),$(ROOT_DIR)/tools)
export BUILD_DIR   := $(ROOT_DIR)/build
export PACKAGE_DIR := $(ROOT_DIR)/build/package
export DIST_DIR    := $(ROOT_DIR)/build/dist

DIRS = $(DL_DIR) $(TOOLS_DIR) $(BUILD_DIR) $(PACKAGE_DIR) $(DIST_DIR)

# Set up default build configurations (debug | release)
GCS_BUILD_CONF		:= release
UAVOGEN_BUILD_CONF	:= release
ANDROIDGCS_BUILD_CONF	:= debug
GOOGLE_API_VERSION	:= 14

# Clean out undesirable variables from the environment and command-line
# to remove the chance that they will cause problems with our build
define SANITIZE_VAR
$(if $(filter-out undefined,$(origin $(1))),
    $(info $(EMPTY) NOTE        Sanitized $(2) variable '$(1)' from $(origin $(1)))
    MAKEOVERRIDES = $(filter-out $(1)=%,$(MAKEOVERRIDES))
    override $(1) :=
    unexport $(1)
)
endef

# These specific variables can influence compilation in unexpected (and undesirable) ways
# gcc flags
SANITIZE_GCC_VARS := TMPDIR GCC_EXEC_PREFIX COMPILER_PATH LIBRARY_PATH
# preprocessor flags
SANITIZE_GCC_VARS += CPATH C_INCLUDE_PATH CPLUS_INCLUDE_PATH OBJC_INCLUDE_PATH DEPENDENCIES_OUTPUT
# make flags
SANITIZE_GCC_VARS += CFLAGS CXXFLAGS CPPFLAGS LDFLAGS LDLIBS
$(foreach var, $(SANITIZE_GCC_VARS), $(eval $(call SANITIZE_VAR,$(var),disallowed)))

# These specific variables used to be valid but now they make no sense
SANITIZE_DEPRECATED_VARS := USE_BOOTLOADER CLEAN_BUILD
$(foreach var, $(SANITIZE_DEPRECATED_VARS), $(eval $(call SANITIZE_VAR,$(var),deprecated)))

# Make sure this isn't being run as root unless installing (no whoami on Windows, but that is ok here)
ifeq ($(shell whoami 2>/dev/null),root)
    ifeq ($(filter install,$(MAKECMDGOALS)),)
        ifndef FAKEROOTKEY
            $(error You should not be running this as root)
        endif
    endif
endif

# Decide on a verbosity level based on the V= parameter
export AT := @
ifndef V
    export V0    :=
    export V1    := $(AT)
else ifeq ($(V), 0)
    export V0    := $(AT)
    export V1    := $(AT)
else ifeq ($(V), 1)
endif

# Make sure we know few things about the architecture before including
# the tools.mk to ensure that we download/install the right tools.
UNAME := $(shell uname)
ARCH  := $(shell uname -m)
# Here and everywhere if not Linux or Mac then assume Windows
ifeq ($(filter Linux Darwin, $(UNAME)), )
    UNAME := Windows
endif

# Include tools installers
include $(ROOT_DIR)/make/tools.mk

# Include third party builders if available
-include $(ROOT_DIR)/make/3rdparty/3rdparty.mk

# We almost need to consider autoconf/automake instead of this
ifeq ($(UNAME), Linux)
    QT_SPEC = linux-g++
    UAVOBJGENERATOR = "$(BUILD_DIR)/uavobjgenerator/uavobjgenerator"
else ifeq ($(UNAME), Darwin)
    QT_SPEC = macx-g++
    UAVOBJGENERATOR = "$(BUILD_DIR)/uavobjgenerator/uavobjgenerator"
else ifeq ($(UNAME), Windows)
    QT_SPEC = win32-g++
    UAVOBJGENERATOR = "$(BUILD_DIR)/uavobjgenerator/$(UAVOGEN_BUILD_CONF)/uavobjgenerator.exe"
endif

##############################
#
# All targets
#
##############################

.PHONY: all
all: uavobjects all_ground all_flight

.PHONY: all_clean
all_clean:
	@$(ECHO) " CLEAN      $(call toprel, $(BUILD_DIR))"
	$(V1) [ ! -d "$(BUILD_DIR)" ] || $(RM) -rf "$(BUILD_DIR)"

.PONY: clean
clean: all_clean


##############################
#
# UAVObjects
#
##############################

ifeq ($(V), 1)
    UAVOGEN_SILENT :=
else
    UAVOGEN_SILENT := silent
endif

UAVOBJGENERATOR_DIR = $(BUILD_DIR)/uavobjgenerator
DIRS += $(UAVOBJGENERATOR_DIR)

.PHONY: uavobjgenerator
uavobjgenerator: | $(UAVOBJGENERATOR_DIR)
	$(V1) cd $(UAVOBJGENERATOR_DIR) && \
	    $(QMAKE) $(ROOT_DIR)/ground/uavobjgenerator/uavobjgenerator.pro \
	    -spec $(QT_SPEC) -r CONFIG+=$(UAVOGEN_BUILD_CONF) CONFIG+=$(UAVOGEN_SILENT) && \
	    $(MAKE) --no-print-directory -w

UAVOBJ_TARGETS := gcs flight python matlab java wireshark

.PHONY: uavobjects
uavobjects:  $(addprefix uavobjects_, $(UAVOBJ_TARGETS))

UAVOBJ_XML_DIR := $(ROOT_DIR)/shared/uavobjectdefinition
UAVOBJ_OUT_DIR := $(BUILD_DIR)/uavobject-synthetics

DIRS += $(UAVOBJ_OUT_DIR)

uavobjects_%: $(UAVOBJ_OUT_DIR) uavobjgenerator
	$(V1) ( cd $(UAVOBJ_OUT_DIR) && \
	    $(UAVOBJGENERATOR) -$* $(UAVOBJ_XML_DIR) $(ROOT_DIR) ; \
	)

uavobjects_test: $(UAVOBJ_OUT_DIR) uavobjgenerator
	$(V1) $(UAVOBJGENERATOR) -v -none $(UAVOBJ_XML_DIR) $(ROOT_DIR)

uavobjects_clean:
	@$(ECHO) " CLEAN      $(call toprel, $(UAVOBJ_OUT_DIR))"
	$(V1) [ ! -d "$(UAVOBJ_OUT_DIR)" ] || $(RM) -r "$(UAVOBJ_OUT_DIR)"

##############################
#
# Flight related components
#
##############################

# Define some pointers to the various important pieces of the flight code
# to prevent these being repeated in every sub makefile
export PIOS          := $(ROOT_DIR)/flight/pios
export FLIGHTLIB     := $(ROOT_DIR)/flight/libraries
export OPMODULEDIR   := $(ROOT_DIR)/flight/modules
export OPUAVOBJ      := $(ROOT_DIR)/flight/uavobjects
export OPUAVTALK     := $(ROOT_DIR)/flight/uavtalk
export OPUAVSYNTHDIR := $(BUILD_DIR)/uavobject-synthetics/flight
export OPGCSSYNTHDIR := $(BUILD_DIR)/openpilotgcs-synthetics

DIRS += $(OPGCSSYNTHDIR)

# Define supported board lists
ALL_BOARDS    := oplinkmini revolution osd revoproto simposix discoveryf4bare gpsplatinum

# Short names of each board (used to display board name in parallel builds)
oplinkmini_short       := 'oplm'
revolution_short       := 'revo'
osd_short              := 'osd '
revoproto_short        := 'revp'
simposix_short         := 'posx'
discoveryf4bare_short  := 'df4b'
gpsplatinum_short      := 'gps9'

# SimPosix only builds on Linux so drop it from the list for
# all other platforms.
ifneq ($(UNAME), Linux)
    ALL_BOARDS := $(filter-out simposix, $(ALL_BOARDS))
endif

# Start out assuming that we'll build fw, bl and bu for all boards
FW_BOARDS  := $(ALL_BOARDS)
BL_BOARDS  := $(ALL_BOARDS)
BU_BOARDS  := $(ALL_BOARDS)
EF_BOARDS  := $(ALL_BOARDS)

# SimPosix doesn't have a BL, BU or EF target so we need to
# filter them out to prevent errors on the all_flight target.
BL_BOARDS  := $(filter-out simposix, $(BL_BOARDS))
BU_BOARDS  := $(filter-out simposix gpsplatinum, $(BU_BOARDS))
EF_BOARDS  := $(filter-out simposix, $(EF_BOARDS))

# Generate the targets for whatever boards are left in each list
FW_TARGETS := $(addprefix fw_, $(FW_BOARDS))
BL_TARGETS := $(addprefix bl_, $(BL_BOARDS))
BU_TARGETS := $(addprefix bu_, $(BU_BOARDS))
EF_TARGETS := $(addprefix ef_, $(EF_BOARDS))

# When building any of the "all_*" targets, tell all sub makefiles to display
# additional details on each line of output to describe which build and target
# that each line applies to. The same applies also to all, opfw_resource,
# package targets
ifneq ($(strip $(filter all_% all opfw_resource package,$(MAKECMDGOALS))),)
    export ENABLE_MSG_EXTRA := yes
endif

# When building more than one goal in a single make invocation, also
# enable the extra context for each output line
ifneq ($(word 2,$(MAKECMDGOALS)),)
    export ENABLE_MSG_EXTRA := yes
endif

# TEMPLATES (used to generate build rules)

# $(1) = Canonical board name all in lower case (e.g. coptercontrol)
# $(2) = Short name for board (e.g cc)
define FW_TEMPLATE
.PHONY: $(1) fw_$(1)
$(1): fw_$(1)_opfw
fw_$(1): fw_$(1)_opfw

fw_$(1)_%: uavobjects_flight
	$(V1) $$(ARM_GCC_VERSION_CHECK_TEMPLATE)
	$(V1) $(MKDIR) -p $(BUILD_DIR)/fw_$(1)/dep
	$(V1) cd $(ROOT_DIR)/flight/targets/boards/$(1)/firmware && \
		$$(MAKE) -r --no-print-directory \
		BUILD_TYPE=fw \
		BOARD_NAME=$(1) \
		BOARD_SHORT_NAME=$(2) \
		TOPDIR=$(ROOT_DIR)/flight/targets/boards/$(1)/firmware \
		OUTDIR=$(BUILD_DIR)/fw_$(1) \
		TARGET=fw_$(1) \
		$$*

.PHONY: $(1)_clean
$(1)_clean: fw_$(1)_clean
fw_$(1)_clean:
	@$(ECHO) " CLEAN      $(call toprel, $(BUILD_DIR)/fw_$(1))"
	$(V1) $(RM) -fr $(BUILD_DIR)/fw_$(1)
endef

# $(1) = Canonical board name all in lower case (e.g. coptercontrol)
# $(2) = Short name for board (e.g cc)
define BL_TEMPLATE
.PHONY: bl_$(1)
bl_$(1): bl_$(1)_bin
bl_$(1)_bino: bl_$(1)_bin

bl_$(1)_%:
	$(V1) $$(ARM_GCC_VERSION_CHECK_TEMPLATE)
	$(V1) $(MKDIR) -p $(BUILD_DIR)/bl_$(1)/dep
	$(V1) cd $(ROOT_DIR)/flight/targets/boards/$(1)/bootloader && \
		$$(MAKE) -r --no-print-directory \
		BUILD_TYPE=bl \
		BOARD_NAME=$(1) \
		BOARD_SHORT_NAME=$(2) \
		TOPDIR=$(ROOT_DIR)/flight/targets/boards/$(1)/bootloader \
		OUTDIR=$(BUILD_DIR)/bl_$(1) \
		TARGET=bl_$(1) \
		$$*

.PHONY: unbrick_$(1)
unbrick_$(1): bl_$(1)_hex
$(if $(filter-out undefined,$(origin UNBRICK_TTY)),
	$(V0) @$(ECHO) " UNBRICK    $(1) via $$(UNBRICK_TTY)"
	$(V1) $(STM32FLASH_DIR)/stm32flash \
		-w $(BUILD_DIR)/bl_$(1)/bl_$(1).hex \
		-g 0x0 \
		$$(UNBRICK_TTY)
,
	$(V0) @$(ECHO)
	$(V0) @$(ECHO) "ERROR: You must specify UNBRICK_TTY=<serial-device> to use for unbricking."
	$(V0) @$(ECHO) "       eg. $$(MAKE) $$@ UNBRICK_TTY=/dev/ttyUSB0"
)

.PHONY: bl_$(1)_clean
bl_$(1)_clean:
	@$(ECHO) " CLEAN      $(call toprel, $(BUILD_DIR)/bl_$(1))"
	$(V1) $(RM) -fr $(BUILD_DIR)/bl_$(1)
endef

# $(1) = Canonical board name all in lower case (e.g. coptercontrol)
# $(2) = Short name for board (e.g cc)
define BU_TEMPLATE
.PHONY: bu_$(1)
bu_$(1): bu_$(1)_opfw

bu_$(1)_%: bl_$(1)_bino
	$(V1) $(MKDIR) -p $(BUILD_DIR)/bu_$(1)/dep
	$(V1) cd $(ROOT_DIR)/flight/targets/common/bootloader_updater && \
		$$(MAKE) -r --no-print-directory \
		BUILD_TYPE=bu \
		BOARD_NAME=$(1) \
		BOARD_SHORT_NAME=$(2) \
		TOPDIR=$(ROOT_DIR)/flight/targets/common/bootloader_updater \
		OUTDIR=$(BUILD_DIR)/bu_$(1) \
		TARGET=bu_$(1) \
		$$*

.PHONY: bu_$(1)_clean
bu_$(1)_clean:
	@$(ECHO) " CLEAN      $(call toprel, $(BUILD_DIR)/bu_$(1))"
	$(V1) $(RM) -fr $(BUILD_DIR)/bu_$(1)
endef

# $(1) = Canonical board name all in lower case (e.g. coptercontrol)
# $(2) = Short name for board (e.g cc)
define EF_TEMPLATE
.PHONY: ef_$(1)
ef_$(1): ef_$(1)_bin

ef_$(1)_%: bl_$(1)_bin fw_$(1)_opfw
	$(V1) $(MKDIR) -p $(BUILD_DIR)/ef_$(1)
	$(V1) cd $(ROOT_DIR)/flight/targets/common/entire_flash && \
		$$(MAKE) -r --no-print-directory \
		BUILD_TYPE=ef \
		BOARD_NAME=$(1) \
		BOARD_SHORT_NAME=$(2) \
		DFU_CMD="$(DFUUTIL_DIR)/bin/dfu-util" \
		TOPDIR=$(ROOT_DIR)/flight/targets/common/entire_flash \
		OUTDIR=$(BUILD_DIR)/ef_$(1) \
		TARGET=ef_$(1) \
		$$*

.PHONY: ef_$(1)_clean
ef_$(1)_clean:
	@$(ECHO) " CLEAN      $(call toprel, $(BUILD_DIR)/ef_$(1))"
	$(V1) $(RM) -fr $(BUILD_DIR)/ef_$(1)
endef

# $(1) = Canonical board name all in lower case (e.g. coptercontrol)
define BOARD_PHONY_TEMPLATE
.PHONY: all_$(1)
all_$(1): $$(filter fw_$(1), $$(FW_TARGETS))
all_$(1): $$(filter bl_$(1), $$(BL_TARGETS))
all_$(1): $$(filter bu_$(1), $$(BU_TARGETS))
all_$(1): $$(filter ef_$(1), $$(EF_TARGETS))

.PHONY: all_$(1)_clean
all_$(1)_clean: $$(addsuffix _clean, $$(filter fw_$(1), $$(FW_TARGETS)))
all_$(1)_clean: $$(addsuffix _clean, $$(filter bl_$(1), $$(BL_TARGETS)))
all_$(1)_clean: $$(addsuffix _clean, $$(filter bu_$(1), $$(BU_TARGETS)))
all_$(1)_clean: $$(addsuffix _clean, $$(filter ef_$(1), $$(EF_TARGETS)))
endef

# Generate flight build rules
.PHONY: all_fw all_fw_clean
all_fw:        $(addsuffix _opfw,  $(FW_TARGETS))
all_fw_clean:  $(addsuffix _clean, $(FW_TARGETS))

.PHONY: all_bl all_bl_clean
all_bl:        $(addsuffix _bin,   $(BL_TARGETS))
all_bl_clean:  $(addsuffix _clean, $(BL_TARGETS))

.PHONY: all_bu all_bu_clean
all_bu:        $(addsuffix _opfw,  $(BU_TARGETS))
all_bu_clean:  $(addsuffix _clean, $(BU_TARGETS))

.PHONY: all_ef all_ef_clean
all_ef:        $(EF_TARGETS)
all_ef_clean:  $(addsuffix _clean, $(EF_TARGETS))

.PHONY: all_flight all_flight_clean
all_flight:       all_fw all_bl all_bu all_ef
all_flight_clean: all_fw_clean all_bl_clean all_bu_clean all_ef_clean

# Expand the groups of targets for each board
$(foreach board, $(ALL_BOARDS), $(eval $(call BOARD_PHONY_TEMPLATE,$(board))))

# Expand the firmware rules
$(foreach board, $(ALL_BOARDS), $(eval $(call FW_TEMPLATE,$(board),$($(board)_short))))

# Expand the bootloader rules
$(foreach board, $(ALL_BOARDS), $(eval $(call BL_TEMPLATE,$(board),$($(board)_short))))

# Expand the bootloader updater rules
$(foreach board, $(ALL_BOARDS), $(eval $(call BU_TEMPLATE,$(board),$($(board)_short))))

# Expand the entire-flash rules
$(foreach board, $(ALL_BOARDS), $(eval $(call EF_TEMPLATE,$(board),$($(board)_short))))

.PHONY: sim_win32
sim_win32: sim_win32_exe

sim_win32_%: uavobjects_flight
	$(V1) $(MKDIR) -p $(BUILD_DIR)/sitl_win32
	$(V1) $(MAKE) --no-print-directory \
		-C $(ROOT_DIR)/flight/targets/OpenPilot --file=$(ROOT_DIR)/flight/targets/OpenPilot/Makefile.win32 $*

.PHONY: sim_osx
sim_osx: sim_osx_elf

sim_osx_%: uavobjects_flight
	$(V1) $(MKDIR) -p $(BUILD_DIR)/sim_osx
	$(V1) $(MAKE) --no-print-directory \
		-C $(ROOT_DIR)/flight/targets/SensorTest --file=$(ROOT_DIR)/flight/targets/SensorTest/Makefile.osx $*

##############################
#
# GCS related components
#
##############################

.PHONY: all_ground
all_ground: openpilotgcs uploader

# Convenience target for the GCS
.PHONY: gcs gcs_qmake gcs_clean
gcs: openpilotgcs
gcs_qmake: openpilotgcs_qmake
gcs_clean: openpilotgcs_clean

ifeq ($(V), 1)
    GCS_SILENT :=
else
    GCS_SILENT := silent
endif

OPENPILOTGCS_DIR := $(BUILD_DIR)/openpilotgcs_$(GCS_BUILD_CONF)
DIRS += $(OPENPILOTGCS_DIR)

OPENPILOTGCS_MAKEFILE := $(OPENPILOTGCS_DIR)/Makefile

.PHONY: openpilotgcs_qmake
openpilotgcs_qmake $(OPENPILOTGCS_MAKEFILE): | $(OPENPILOTGCS_DIR)
	$(V1) cd $(OPENPILOTGCS_DIR) && \
	    $(QMAKE) $(ROOT_DIR)/ground/openpilotgcs/openpilotgcs.pro \
	    -spec $(QT_SPEC) -r CONFIG+=$(GCS_BUILD_CONF) CONFIG+=$(GCS_SILENT) $(GCS_QMAKE_OPTS)

.PHONY: openpilotgcs
openpilotgcs: uavobjects_gcs $(OPENPILOTGCS_MAKEFILE)
	$(V1) $(MAKE) -w -C $(OPENPILOTGCS_DIR)/$(MAKE_DIR);

.PHONY: openpilotgcs_clean
openpilotgcs_clean:
	@$(ECHO) " CLEAN      $(call toprel, $(OPENPILOTGCS_DIR))"
	$(V1) [ ! -d "$(OPENPILOTGCS_DIR)" ] || $(RM) -r "$(OPENPILOTGCS_DIR)"



################################
#
# Serial Uploader tool
#
################################

UPLOADER_DIR := $(BUILD_DIR)/uploader_$(GCS_BUILD_CONF)
DIRS += $(UPLOADER_DIR)

UPLOADER_MAKEFILE := $(UPLOADER_DIR)/Makefile

.PHONY: uploader_qmake
uploader_qmake $(UPLOADER_MAKEFILE): | $(UPLOADER_DIR)
	$(V1) cd $(UPLOADER_DIR) && \
	    $(QMAKE) $(ROOT_DIR)/ground/openpilotgcs/src/experimental/USB_UPLOAD_TOOL/upload.pro \
	    -spec $(QT_SPEC) -r CONFIG+=$(GCS_BUILD_CONF) CONFIG+=$(GCS_SILENT) $(GCS_QMAKE_OPTS)

.PHONY: uploader
uploader: $(UPLOADER_MAKEFILE)
	$(V1) $(MAKE) -w -C $(UPLOADER_DIR)

.PHONY: uploader_clean
uploader_clean:
	@$(ECHO) " CLEAN      $(call toprel, $(UPLOADER_DIR))"
	$(V1) [ ! -d "$(UPLOADER_DIR)" ] || $(RM) -r "$(UPLOADER_DIR)"


# We want to take snapshots of the UAVOs at each point that they change
# to allow the GCS to be compatible with as many versions as possible.
# We always include a pseudo collection called "srctree" which represents
# the UAVOs in the source tree. So not necessary to add current tree UAVO
# hash here, it is always included.

# Find the git hashes of each commit that changes uavobjects with:
#   git log --format=%h -- shared/uavobjectdefinition/ | head -n 2
# List only UAVO hashes of past releases, do not list current hash.
# Past compatible versions are so far: RELEASE-12.10.2
UAVO_GIT_VERSIONS := 5e14f53

# All versions includes also the current source tree UAVO hash
UAVO_ALL_VERSIONS := $(UAVO_GIT_VERSIONS) srctree

# This is where the UAVO collections are stored
UAVO_COLLECTION_DIR := $(BUILD_DIR)/uavo-collections

# $(1) git hash of a UAVO snapshot
define UAVO_COLLECTION_GIT_TEMPLATE

# Make the output directory that will contain all of the synthetics for the
# uavo collection referenced by the git hash $(1)
$$(UAVO_COLLECTION_DIR)/$(1):
	$$(V1) $(MKDIR) -p $$(UAVO_COLLECTION_DIR)/$(1)

# Extract the snapshot of shared/uavobjectdefinition from git hash $(1)
$$(UAVO_COLLECTION_DIR)/$(1)/uavo-xml.tar: | $$(UAVO_COLLECTION_DIR)/$(1)
$$(UAVO_COLLECTION_DIR)/$(1)/uavo-xml.tar:
	$$(V0) @$(ECHO) " UAVOTAR   $(1)"
	$$(V1) $(GIT) archive $(1) -o $$@ -- shared/uavobjectdefinition/

# Extract the uavo xml files from our snapshot
$$(UAVO_COLLECTION_DIR)/$(1)/uavo-xml: $$(UAVO_COLLECTION_DIR)/$(1)/uavo-xml.tar
	$$(V0) @$(ECHO) " UAVOUNTAR $(1)"
	$$(V1) $(RM) -rf $$@
	$$(V1) $(MKDIR) -p $$@
	$$(V1) $(TAR) -C $$(call toprel, $$@) -xf $$(call toprel, $$<) || $(RM) -rf $$@
endef

# Map the current working directory into the set of UAVO collections
$(UAVO_COLLECTION_DIR)/srctree:
	$(V1) $(MKDIR) -p $@

$(UAVO_COLLECTION_DIR)/srctree/uavo-xml: | $(UAVO_COLLECTION_DIR)/srctree
$(UAVO_COLLECTION_DIR)/srctree/uavo-xml: $(UAVOBJ_XML_DIR)
	$(V1) $(LN) -sf $(ROOT_DIR) $(UAVO_COLLECTION_DIR)/srctree/uavo-xml

# $(1) git hash (or symbolic name) of a UAVO snapshot
define UAVO_COLLECTION_BUILD_TEMPLATE

# This leaves us with a (broken) symlink that points to the full sha1sum of the collection
$$(UAVO_COLLECTION_DIR)/$(1)/uavohash: $$(UAVO_COLLECTION_DIR)/$(1)/uavo-xml
        # Compute the sha1 hash for this UAVO collection
        # The sed bit truncates the UAVO hash to 16 hex digits
	$$(V1) $$(VERSION_INFO) \
			--uavodir=$$(UAVO_COLLECTION_DIR)/$(1)/uavo-xml/shared/uavobjectdefinition \
			--format='$$$${UAVO_HASH}' | \
		$(SED) -e 's|\(................\).*|\1|' > $$@

	$$(V0) @$(ECHO) " UAVOHASH  $(1) ->" $$$$(cat $$(UAVO_COLLECTION_DIR)/$(1)/uavohash)

# Generate the java uavobjects for this UAVO collection
$$(UAVO_COLLECTION_DIR)/$(1)/java-build/java: $$(UAVO_COLLECTION_DIR)/$(1)/uavohash
	$$(V0) @$(ECHO) " UAVOJAVA  $(1)   " $$$$(cat $$(UAVO_COLLECTION_DIR)/$(1)/uavohash)
	$$(V1) $(MKDIR) -p $$@
	$$(V1) ( \
		cd $$(UAVO_COLLECTION_DIR)/$(1)/java-build && \
		$$(UAVOBJGENERATOR) -java $$(UAVO_COLLECTION_DIR)/$(1)/uavo-xml/shared/uavobjectdefinition $$(ROOT_DIR) ; \
	)

# Build a jar file for this UAVO collection
$$(UAVO_COLLECTION_DIR)/$(1)/java-build/uavobjects.jar: | $$(ANDROIDGCS_ASSETS_DIR)/uavos
$$(UAVO_COLLECTION_DIR)/$(1)/java-build/uavobjects.jar: $$(UAVO_COLLECTION_DIR)/$(1)/java-build/java
	$$(V0) @$(ECHO) " UAVOJAR   $(1)   " $$$$(cat $$(UAVO_COLLECTION_DIR)/$(1)/uavohash)
	$$(V1) ( \
		HASH=$$$$(cat $$(UAVO_COLLECTION_DIR)/$(1)/uavohash) && \
		cd $$(UAVO_COLLECTION_DIR)/$(1)/java-build && \
		$(JAVAC) java/*.java \
			$$(ROOT_DIR)/androidgcs/src/org/openpilot/uavtalk/UAVDataObject.java \
			$$(ROOT_DIR)/androidgcs/src/org/openpilot/uavtalk/UAVObject*.java \
			$$(ROOT_DIR)/androidgcs/src/org/openpilot/uavtalk/UAVMetaObject.java \
			-d . && \
		find ./org/openpilot/uavtalk/uavobjects -type f -name '*.class' > classlist.txt && \
		$(JAR) cf tmp_uavobjects.jar @classlist.txt && \
		$$(ANDROID_DX) \
			--dex \
			--output $$(ANDROIDGCS_ASSETS_DIR)/uavos/$$$${HASH}.jar \
			tmp_uavobjects.jar && \
		$(LN) -sf $$(ANDROIDGCS_ASSETS_DIR)/uavos/$$$${HASH}.jar uavobjects.jar \
	)

endef

# One of these for each element of UAVO_GIT_VERSIONS so we can extract the UAVOs from git
$(foreach githash, $(UAVO_GIT_VERSIONS), $(eval $(call UAVO_COLLECTION_GIT_TEMPLATE,$(githash))))

# One of these for each UAVO_ALL_VERSIONS which includes the ones in the srctree
$(foreach githash, $(UAVO_ALL_VERSIONS), $(eval $(call UAVO_COLLECTION_BUILD_TEMPLATE,$(githash))))

.PHONY: uavo-collections_java
uavo-collections_java: $(foreach githash, $(UAVO_ALL_VERSIONS), $(UAVO_COLLECTION_DIR)/$(githash)/java-build/uavobjects.jar)

.PHONY: uavo-collections
uavo-collections: uavo-collections_java

.PHONY: uavo-collections_clean
uavo-collections_clean:
	@$(ECHO) " CLEAN      $(call toprel, $(UAVO_COLLECTION_DIR))"
	$(V1) [ ! -d "$(UAVO_COLLECTION_DIR)" ] || $(RM) -r $(UAVO_COLLECTION_DIR)

##############################
#
# Unit Tests
#
##############################

ALL_UNITTESTS := logfs math lednotification

# Build the directory for the unit tests
UT_OUT_DIR := $(BUILD_DIR)/unit_tests
DIRS += $(UT_OUT_DIR)

.PHONY: all_ut
all_ut: $(addsuffix _elf, $(addprefix ut_, $(ALL_UNITTESTS)))

.PHONY: all_ut_xml
all_ut_xml: $(addsuffix _xml, $(addprefix ut_, $(ALL_UNITTESTS)))

.PHONY: all_ut_run
all_ut_run: $(addsuffix _run, $(addprefix ut_, $(ALL_UNITTESTS)))

.PHONY: all_ut_clean
all_ut_clean:
	@$(ECHO) " CLEAN      $(call toprel, $(UT_OUT_DIR))"
	$(V1) [ ! -d "$(UT_OUT_DIR)" ] || $(RM) -r "$(UT_OUT_DIR)"

# $(1) = Unit test name
define UT_TEMPLATE
.PHONY: ut_$(1)
ut_$(1): ut_$(1)_run

ut_$(1)_%: $$(UT_OUT_DIR)
	$(V1) $(MKDIR) -p $(UT_OUT_DIR)/$(1)
	$(V1) cd $(ROOT_DIR)/flight/tests/$(1) && \
		$$(MAKE) -r --no-print-directory \
		BUILD_TYPE=ut \
		BOARD_SHORT_NAME=$(1) \
		TOPDIR=$(ROOT_DIR)/flight/tests/$(1) \
		OUTDIR="$(UT_OUT_DIR)/$(1)" \
		TARGET=$(1) \
		$$*

.PHONY: ut_$(1)_clean
ut_$(1)_clean:
	@$(ECHO) " CLEAN      $(call toprel, $(UT_OUT_DIR)/$(1))"
	$(V1) [ ! -d "$(UT_OUT_DIR)/$(1)" ] || $(RM) -r "$(UT_OUT_DIR)/$(1)"
endef

# Expand the unittest rules
$(foreach ut, $(ALL_UNITTESTS), $(eval $(call UT_TEMPLATE,$(ut))))

# Disable parallel make when the all_ut_run target is requested otherwise the TAP
# output is interleaved with the rest of the make output.
ifneq ($(strip $(filter all_ut_run,$(MAKECMDGOALS))),)
.NOTPARALLEL:
    $(info $(EMPTY) NOTE        Parallel make disabled by all_ut_run target so we have sane console output)
endif

##############################
#
# Packaging components
#
##############################

# Firmware files to package
PACKAGE_FW_EXCLUDE  := fw_simposix $(if $(PACKAGE_FW_INCLUDE_DISCOVERYF4BARE),,fw_discoveryf4bare)
PACKAGE_FW_TARGETS  := $(filter-out $(PACKAGE_FW_EXCLUDE), $(FW_TARGETS))
PACKAGE_ELF_TARGETS := $(filter     fw_simposix, $(FW_TARGETS))

# Rules to generate GCS resources used to embed firmware binaries into the GCS.
# They are used later by the vehicle setup wizard to update board firmware.
# To open a firmware image use ":/firmware/fw_coptercontrol.opfw"
OPFW_RESOURCE := $(OPGCSSYNTHDIR)/opfw_resource.qrc
OPFW_RESOURCE_PREFIX := ../../
OPFW_FILES := $(foreach fw_targ, $(PACKAGE_FW_TARGETS), $(call toprel, $(BUILD_DIR)/$(fw_targ)/$(fw_targ).opfw))
OPFW_CONTENTS := \
<!DOCTYPE RCC><RCC version="1.0"> \
    <qresource prefix="/firmware"> \
        $(foreach fw_file, $(OPFW_FILES), <file alias="$(notdir $(fw_file))">$(OPFW_RESOURCE_PREFIX)$(fw_file)</file>) \
    </qresource> \
</RCC>

.PHONY: opfw_resource
opfw_resource: $(OPFW_RESOURCE)

$(OPFW_RESOURCE): $(FW_TARGETS) | $(OPGCSSYNTHDIR)
	@$(ECHO) Generating OPFW resource file $(call toprel, $@)
	$(V1) $(ECHO) $(QUOTE)$(OPFW_CONTENTS)$(QUOTE) > $@

# If opfw_resource or all firmware are requested, GCS should depend on the resource
ifneq ($(strip $(filter opfw_resource all all_fw all_flight package,$(MAKECMDGOALS))),)
$(OPENPILOTGCS_MAKEFILE): $(OPFW_RESOURCE)
endif

# Packaging targets: package
#  - builds all firmware, opfw_resource, gcs
#  - copies firmware into a package directory
#  - calls paltform-specific packaging script

# Define some variables
PACKAGE_LBL       := $(shell $(VERSION_INFO) --format=\$${LABEL})
PACKAGE_NAME      := OpenPilot
PACKAGE_SEP       := -
PACKAGE_FULL_NAME := $(PACKAGE_NAME)$(PACKAGE_SEP)$(PACKAGE_LBL)

include $(ROOT_DIR)/package/$(UNAME).mk

# Source distribution is never dirty because it uses git archive
DIST_NAME := $(DIST_DIR)/$(subst dirty-,,$(PACKAGE_FULL_NAME)).tar

##############################
#
# Source code formatting
#
##############################

UNCRUSTIFY_TARGETS := flight ground

# $(1) = Uncrustify target (e.g flight or ground)
# $(2) = Target root directory
define UNCRUSTIFY_TEMPLATE

.PHONY: uncrustify_$(1)
uncrustify_$(1):
	@$(ECHO) "Auto-formatting $(1) source code"
	$(V1) UNCRUSTIFY_CONFIG="$(ROOT_DIR)/make/uncrustify/uncrustify.cfg" $(SHELL) make/scripts/uncrustify.sh $(call toprel, $(2))
endef

$(foreach uncrustify_targ, $(UNCRUSTIFY_TARGETS), $(eval $(call UNCRUSTIFY_TEMPLATE,$(uncrustify_targ),$(ROOT_DIR)/$(uncrustify_targ))))

.PHONY: uncrustify_all
uncrustify_all: $(addprefix uncrustify_,$(UNCRUSTIFY_TARGETS))

##############################
#
# Doxygen documentation
#
# Each target should have own Doxyfile.$(target) with build directory build/docs/$(target),
# proper source directory (e.g. $(target)) and appropriate other doxygen options.
#
##############################

DOCS_TARGETS := flight ground uavobjects

# $(1) = Doxygen target (e.g flight or ground)
define DOXYGEN_TEMPLATE

.PHONY: docs_$(1)
docs_$(1): docs_$(1)_clean
	@$(ECHO) "Generating $(1) documentation"
	$(V1) $(MKDIR) -p $(BUILD_DIR)/docs/$(1)
	$(V1) $(DOXYGEN) $(ROOT_DIR)/make/doxygen/Doxyfile.$(1)

.PHONY: docs_$(1)_clean
docs_$(1)_clean:
	@$(ECHO) " CLEAN      $(call toprel, $(BUILD_DIR)/docs/$(1))"
	$(V1) [ ! -d "$(BUILD_DIR)/docs/$(1)" ] || $(RM) -r "$(BUILD_DIR)/docs/$(1)"

endef

$(foreach docs_targ, $(DOCS_TARGETS), $(eval $(call DOXYGEN_TEMPLATE,$(docs_targ))))

.PHONY: docs_all
docs_all: $(addprefix docs_,$(DOCS_TARGETS))

.PHONY: docs_all_clean
docs_all_clean:
	@$(ECHO) " CLEAN      $(call toprel, $(BUILD_DIR)/docs)"
	$(V1) [ ! -d "$(BUILD_DIR)/docs" ] || $(RM) -rf "$(BUILD_DIR)/docs"

##############################
#
# Build info
#
##############################

.PHONY: build-info
build-info: | $(BUILD_DIR)
	@$(ECHO) " BUILD-INFO $(call toprel, $(BUILD_DIR)/$@.txt)"
	$(V1) $(VERSION_INFO) \
		--uavodir=$(ROOT_DIR)/shared/uavobjectdefinition \
		--template="make/templates/$@.txt" \
		--outfile="$(BUILD_DIR)/$@.txt"

##############################
#
# Source for distribution
#
##############################

DIST_VER_INFO := $(DIST_DIR)/version-info.json

$(DIST_VER_INFO): .git/index | $(DIST_DIR)
	$(V1) $(VERSION_INFO) --jsonpath="$(DIST_DIR)"


$(DIST_NAME).gz: $(DIST_VER_INFO) .git/index | $(DIST_DIR)
	@$(ECHO) " SOURCE FOR DISTRIBUTION $(call toprel, $(DIST_NAME).gz)"
	$(V1) git archive --prefix="$(PACKAGE_NAME)/" -o "$(DIST_NAME)" HEAD
	$(V1) tar --append --file="$(DIST_NAME)" \
		--transform='s,.*version-info.json,$(PACKAGE_NAME)/version-info.json,' \
		$(call toprel, "$(DIST_VER_INFO)")
	$(V1) gzip -f "$(DIST_NAME)"

.PHONY: dist
dist: $(DIST_NAME).gz


##############################
#
# Directories
#
##############################

$(DIRS):
	$(V1) $(MKDIR) -p $@


##############################
#
# Help message, the default Makefile goal
#
##############################

.DEFAULT_GOAL := help

.PHONY: help
help:
	@$(ECHO)
	@$(ECHO) "   This Makefile is known to work on Linux and Mac in a standard shell environment."
	@$(ECHO) "   It also works on Windows by following the instructions given on this wiki page:"
	@$(ECHO) "       http://wiki.openpilot.org/display/Doc/Windows%3A+Building+and+Packaging"
	@$(ECHO)
	@$(ECHO) "   Here is a summary of the available targets:"
	@$(ECHO)
	@$(ECHO) "   [Source tree preparation]"
	@$(ECHO) "     prepare              - Install GIT commit message template"
	@$(ECHO) "   [Tool Installers]"
	@$(ECHO) "     arm_sdk_install      - Install the GNU ARM gcc toolchain"
	@$(ECHO) "     qt_sdk_install       - Install the QT development tools"
	@$(ECHO) "     nsis_install         - Install the NSIS Unicode (Windows only)"
	@$(ECHO) "     sdl_install          - Install the SDL library (Windows only)"
	@$(ECHO) "     mesawin_install      - Install the OpenGL32 DLL (Windows only)"
	@$(ECHO) "     openssl_install      - Install the OpenSSL libraries (Windows only)"
	@$(ECHO) "     uncrustify_install   - Install the Uncrustify source code beautifier"
	@$(ECHO) "     doxygen_install      - Install the Doxygen documentation generator"
	@$(ECHO) "     gtest_install        - Install the GoogleTest framework"
	@$(ECHO) "   These targets are not updated yet and are probably broken:"
	@$(ECHO) "     openocd_install      - Install the OpenOCD JTAG daemon"
	@$(ECHO) "     stm32flash_install   - Install the stm32flash tool for unbricking F1-based boards"
	@$(ECHO) "     dfuutil_install      - Install the dfu-util tool for unbricking F4-based boards"
	@$(ECHO) "   Install all available tools:"
	@$(ECHO) "     all_sdk_install      - Install all of above (platform-dependent)"
	@$(ECHO) "     build_sdk_install    - Install only essential for build tools (platform-dependent)"
	@$(ECHO)
	@$(ECHO) "   Other tool options are:"
	@$(ECHO) "     <tool>_version       - Display <tool> version"
	@$(ECHO) "     <tool>_clean         - Remove installed <tool>"
	@$(ECHO) "     <tool>_distclean     - Remove downloaded <tool> distribution file(s)"
	@$(ECHO)
	@$(ECHO) "   [Big Hammer]"
	@$(ECHO) "     all                  - Generate UAVObjects, build openpilot firmware and gcs"
	@$(ECHO) "     all_flight           - Build all firmware, bootloaders and bootloader updaters"
	@$(ECHO) "     all_fw               - Build only firmware for all boards"
	@$(ECHO) "     all_bl               - Build only bootloaders for all boards"
	@$(ECHO) "     all_bu               - Build only bootloader updaters for all boards"
	@$(ECHO)
	@$(ECHO) "     all_clean            - Remove your build directory ($(BUILD_DIR))"
	@$(ECHO) "     all_flight_clean     - Remove all firmware, bootloaders and bootloader updaters"
	@$(ECHO) "     all_fw_clean         - Remove firmware for all boards"
	@$(ECHO) "     all_bl_clean         - Remove bootloaders for all boards"
	@$(ECHO) "     all_bu_clean         - Remove bootloader updaters for all boards"
	@$(ECHO)
	@$(ECHO) "     all_<board>          - Build all available images for <board>"
	@$(ECHO) "     all_<board>_clean    - Remove all available images for <board>"
	@$(ECHO)
	@$(ECHO) "     all_ut               - Build all unit tests"
	@$(ECHO) "     all_ut_tap           - Run all unit tests and capture all TAP output to files"
	@$(ECHO) "     all_ut_run           - Run all unit tests and dump TAP output to console"
	@$(ECHO)
	@$(ECHO) "   [Firmware]"
	@$(ECHO) "     <board>              - Build firmware for <board>"
	@$(ECHO) "                            Supported boards are ($(ALL_BOARDS))"
	@$(ECHO) "     fw_<board>           - Build firmware for <board>"
	@$(ECHO) "                            Supported boards are ($(FW_BOARDS))"
	@$(ECHO) "     fw_<board>_clean     - Remove firmware for <board>"
	@$(ECHO) "     fw_<board>_program   - Use OpenOCD + JTAG to write firmware to <board>"
	@$(ECHO)
	@$(ECHO) "   [Bootloader]"
	@$(ECHO) "     bl_<board>           - Build bootloader for <board>"
	@$(ECHO) "                            Supported boards are ($(BL_BOARDS))"
	@$(ECHO) "     bl_<board>_clean     - Remove bootloader for <board>"
	@$(ECHO) "     bl_<board>_program   - Use OpenOCD + JTAG to write bootloader to <board>"
	@$(ECHO)
	@$(ECHO) "   [Entire Flash]"
	@$(ECHO) "     ef_<board>           - Build entire flash image for <board>"
	@$(ECHO) "                            Supported boards are ($(EF_BOARDS))"
	@$(ECHO) "     ef_<board>_clean     - Remove entire flash image for <board>"
	@$(ECHO) "     ef_<board>_program   - Use OpenOCD + JTAG to write entire flash image to <board>"
	@$(ECHO)
	@$(ECHO) "   [Bootloader Updater]"
	@$(ECHO) "     bu_<board>           - Build bootloader updater for <board>"
	@$(ECHO) "                            Supported boards are ($(BU_BOARDS))"
	@$(ECHO) "     bu_<board>_clean     - Remove bootloader updater for <board>"
	@$(ECHO)
	@$(ECHO) "   [Unbrick a board]"
	@$(ECHO) "     unbrick_<board>      - Use the STM32's built in boot ROM to write a bootloader to <board>"
	@$(ECHO) "                            Supported boards are ($(BL_BOARDS))"
	@$(ECHO) "   [Unittests]"
	@$(ECHO) "     ut_<test>            - Build unit test <test>"
	@$(ECHO) "     ut_<test>_xml        - Run test and capture XML output into a file"
	@$(ECHO) "     ut_<test>_run        - Run test and dump output to console"
	@$(ECHO)
	@$(ECHO) "   [Simulation]"
	@$(ECHO) "     sim_osx              - Build OpenPilot simulation firmware for OSX"
	@$(ECHO) "     sim_osx_clean        - Delete all build output for the osx simulation"
	@$(ECHO) "     sim_win32            - Build OpenPilot simulation firmware for Windows"
	@$(ECHO) "                            using mingw and msys"
	@$(ECHO) "     sim_win32_clean      - Delete all build output for the win32 simulation"
	@$(ECHO)
	@$(ECHO) "   [GCS]"
	@$(ECHO) "     gcs                  - Build the Ground Control System (GCS) application (debug|release)"
	@$(ECHO) "                            Compile specific directory: MAKE_DIR=<dir>"
	@$(ECHO) "                            Example: make gcs MAKE_DIR=src/plugins/coreplugin"
	@$(ECHO) "     gcs_qmake            - Run qmake for the Ground Control System (GCS) application (debug|release)"
	@$(ECHO) "     gcs_clean            - Remove the Ground Control System (GCS) application (debug|release)"
	@$(ECHO) "                            Supported build configurations: GCS_BUILD_CONF=debug|release (default is $(GCS_BUILD_CONF))"
	@$(ECHO)
	@$(ECHO) "   [Uploader Tool]"
	@$(ECHO) "     uploader             - Build the serial uploader tool (debug|release)"
	@$(ECHO) "     uploader_qmake       - Run qmake for the serial uploader tool (debug|release)"
	@$(ECHO) "     uploader_clean       - Remove the serial uploader tool (debug|release)"
	@$(ECHO) "                            Supported build configurations: GCS_BUILD_CONF=debug|release (default is $(GCS_BUILD_CONF))"
	@$(ECHO)
	@$(ECHO)
	@$(ECHO) "   [UAVObjects]"
	@$(ECHO) "     uavobjects           - Generate source files from the UAVObject definition XML files"
	@$(ECHO) "     uavobjects_test      - Parse xml-files - check for valid, duplicate ObjId's, ..."
	@$(ECHO) "     uavobjects_<group>   - Generate source files from a subset of the UAVObject definition XML files"
	@$(ECHO) "                            Supported groups are ($(UAVOBJ_TARGETS))"
	@$(ECHO)
	@$(ECHO) "   [Packaging]"
	@$(ECHO) "     package              - Build and package the OpenPilot platform-dependent package (no clean)"
	@$(ECHO) "     opfw_resource        - Generate resources to embed firmware binaries into the GCS"
	@$(ECHO) "     dist                 - Generate source archive for distribution"
	@$(ECHO) "     install              - Install GCS to \"DESTDIR\" with prefix \"prefix\" (Linux only)"
	@$(ECHO)
	@$(ECHO) "   [Code Formatting]"
	@$(ECHO) "     uncrustify_<source>  - Reformat <source> code according to the project's standards"
	@$(ECHO) "                            Supported sources are ($(UNCRUSTIFY_TARGETS))"
	@$(ECHO) "     uncrustify_all       - Reformat all source code"
	@$(ECHO)
	@$(ECHO) "   [Code Documentation]"
	@$(ECHO) "     docs_<source>        - Generate HTML documentation for <source>"
	@$(ECHO) "                            Supported sources are ($(DOCS_TARGETS))"
	@$(ECHO) "     docs_all             - Generate HTML documentation for all"
	@$(ECHO) "     docs_<source>_clean  - Delete generated documentation for <source>"
	@$(ECHO) "     docs_all_clean       - Delete all generated documentation"
	@$(ECHO)
	@$(ECHO) "   Hint: Add V=1 to your command line to see verbose build output."
	@$(ECHO)
	@$(ECHO) "  Notes: All tool distribution files will be downloaded into $(DL_DIR)"
	@$(ECHO) "         All tools will be installed into $(TOOLS_DIR)"
	@$(ECHO) "         All build output will be placed in $(BUILD_DIR)"
	@$(ECHO)
	@$(ECHO) "  Tool download and install directories can be changed using environment variables:"
	@$(ECHO) "         OPENPILOT_DL_DIR        full path to downloads directory [downloads if not set]"
	@$(ECHO) "         OPENPILOT_TOOLS_DIR     full path to installed tools directory [tools if not set]"
	@$(ECHO) "  More info: http://wiki.openpilot.org/display/Doc/OpenPilot+Build+System+Overview"
	@$(ECHO)
