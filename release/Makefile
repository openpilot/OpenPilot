# Set up a default goal
.DEFAULT_GOAL := help

# Tried the best to support parallel (-j) builds. But since this Makefile
# uses other Makefiles to build few targets which in turn have similar
# dependencies on uavobjects and other generated files, it is difficult
# to support parallel builds perfectly. But at least it was tested with
# -j (unlimited job number) on Windows and Linux.

# Locate the root of the tree
WHEREAMI := $(dir $(lastword $(MAKEFILE_LIST)))
ROOT_DIR := $(realpath $(WHEREAMI)/../)

# Set up some macros
BUILD_DIR    := $(ROOT_DIR)/build
VERSION_CMD  := python $(ROOT_DIR)/make/scripts/version-info.py --path="$(ROOT_DIR)"
RELEASE_LBL  := $(shell $(VERSION_CMD) --format=\$${DATE}-\$${TAG_OR_HASH8}\$${DIRTY})
RELEASE_DIR  := $(BUILD_DIR)/release-$(RELEASE_LBL)
FW_DIR       := $(RELEASE_DIR)/firmware-$(RELEASE_LBL)
BL_DIR       := $(FW_DIR)/bootloaders
BLUPD_DIR    := $(FW_DIR)/bootloader_updaters

# Setup targets
FW_TARGETS_COMMON := ahrs pipxtreme
FW_TARGETS_INPUT  := coptercontrol openpilot
FW_TARGETS_TOOLS  := coptercontrol
FW_TARGETS        := $(FW_TARGETS_COMMON) $(FW_TARGETS_INPUT)
BL_TARGETS        := $(addprefix bl_, $(FW_TARGETS))
BLUPD_TARGETS     := $(addprefix blupd_, $(FW_TARGETS))

help:
	@echo
	@echo "   This Makefile is known to work on Linux and Mac in a standard shell environment."
	@echo "   It also works on Windows by following the instructions in ../make/winx86/README.txt."
	@echo
	@echo "   Here is a summary of the available targets:"
	@echo
	@echo "   [Release build and packaging]"
	@echo "     release              - Build and package the OpenPilot release"
	@echo "     release_flight       - Build and package the OpenPilot flight firmware"
	@echo
	@echo "   Notes:"
	@echo "     - the build directory will be removed first on every run"
	@echo "       unless CLEAN_BUILD=NO is defined (recommended for testing only)"
	@echo "     - release packages will be placed in $(RELEASE_DIR)"
	@echo

# Clean and build uavobjects since all parts depend on them
uavobjects: all_clean
	$(V1) $(MAKE) -C $(ROOT_DIR) $@

all_clean:
ifneq ($(CLEAN_BUILD), NO)
	$(V1) $(MAKE) -C $(ROOT_DIR) $@
endif

# Install template:
#   $1 = target
#   $2 = dependencies
#   $3 = install directory (must be defined)
#   $4 = installed file name prefix (optional)
#   $5 = installed file name suffix (optional)
#   $6 = extra make options (for instance, USE_SPEKTRUM=YES)
#   $7 = optional target suffix (for instance, clean, if target must be cleaned first)
#   $8 = list of targets to install (without _install suffix)
#   $9 = inner make target (usually install, but can be other to just build)
define INSTALL_TEMPLATE
$(1): $(2)
ifneq ($(7),)
	$$(V1) +$(MAKE) -C $(ROOT_DIR) $(6) $(addsuffix _$(7), $(8))
endif
	$$(V1) +$(MAKE) -C $(ROOT_DIR) INSTALL_DIR=$(3) INSTALL_PFX=$(4) INSTALL_SFX=$(5) $(6) $(addsuffix _$(9), $(8))
.PHONY: $(1)
endef

# Firmware for different input drivers
$(eval $(call INSTALL_TEMPLATE,fw_common,uavobjects,$(FW_DIR),,-$(RELEASE_LBL),,,$(FW_TARGETS_COMMON),install))
$(eval $(call INSTALL_TEMPLATE,fw_pwm,uavobjects,$(FW_DIR),,-pwm-$(RELEASE_LBL),,clean,$(FW_TARGETS_INPUT),install))
$(eval $(call INSTALL_TEMPLATE,fw_spektrum,uavobjects,$(FW_DIR),,-spektrum-$(RELEASE_LBL),USE_SPEKTRUM=YES,clean,$(FW_TARGETS_INPUT),install))
$(eval $(call INSTALL_TEMPLATE,fw_ppm,uavobjects,$(FW_DIR),,-ppm-$(RELEASE_LBL),USE_PPM=YES,clean,$(FW_TARGETS_INPUT),install))

# Bootloaders (change 'bin' to 'install' to install bootloaders too)
$(eval $(call INSTALL_TEMPLATE,all_bl,uavobjects,$(BL_DIR),,-$(RELEASE_LBL),,,$(BL_TARGETS),bin))

# Bootloader Updaters
$(eval $(call INSTALL_TEMPLATE,blupd_coptercontrol,all_bl,$(BLUPD_DIR),CopterControl_,-$(RELEASE_LBL),,,blupd_coptercontrol,install))
$(eval $(call INSTALL_TEMPLATE,blupd_ahrs,all_bl,$(BLUPD_DIR),AHRS_,-$(RELEASE_LBL),,,blupd_ahrs,install))
$(eval $(call INSTALL_TEMPLATE,blupd_openpilot,all_bl,$(BLUPD_DIR),OpenPilot_,-$(RELEASE_LBL),,,blupd_openpilot,install))
$(eval $(call INSTALL_TEMPLATE,blupd_pipxtreme,all_bl,$(BLUPD_DIR),PipXtreme_,-$(RELEASE_LBL),,,blupd_pipxtreme,install))

# CopterControl flash eraser tool
$(eval $(call INSTALL_TEMPLATE,fw_tools,uavobjects,$(BLUPD_DIR),,-FlashEraser-$(RELEASE_LBL),ERASE_FLASH=YES,clean,$(FW_TARGETS_TOOLS),install))

# Order-only dependencies
# They are bit complicated to support parallel (-j) builds and to create
# the pwm/ppm/spektrum and CC flash eraser targets in some fixed order

fw_pwm: |		# default dependencies, will be built first

fw_spektrum: | fw_pwm	# ordered build

fw_ppm: | fw_spektrum   # ordered build

fw_tools: | fw_spektrum # ordered build, replace fw_spektrum by fw_ppm if uncommented below

release_fw: | fw_common fw_pwm fw_spektrum # fw_ppm

release_blupd: | $(BLUPD_TARGETS)

release_flight: | release_fw release_blupd fw_tools

release_ground: | ground_package

release: | release_flight release_ground

.PHONY: help uavobjects all_clean release release_flight release_fw release_blupd release_ground

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

ifneq ($(V),1)
MAKEFLAGS += --no-print-directory
endif

# Platform-dependent stuff
PLATFORM := winx86
UNAME := $(shell uname)
ifeq ($(UNAME), Linux)
  PLATFORM := linux
endif
ifeq ($(UNAME), Darwin)
  PLATFORM := osx
endif

include $(WHEREAMI)/Makefile.$(PLATFORM)
