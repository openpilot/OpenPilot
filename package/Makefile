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
PACKAGE_LBL  := $(shell $(VERSION_CMD) --format=\$${DATE}-\$${TAG_OR_HASH8}\$${DIRTY})
PACKAGE_DIR  := $(BUILD_DIR)/package-$(PACKAGE_LBL)
FW_DIR       := $(PACKAGE_DIR)/firmware-$(PACKAGE_LBL)
BL_DIR       := $(FW_DIR)/bootloaders
BU_DIR       := $(FW_DIR)/bootloader-updaters
FE_DIR       := $(FW_DIR)/flash-erase-tools

# Clean build options (recommended for package testing only)
ifeq ($(CLEAN_BUILD), NO)
CLEAN_GROUND := NO
CLEAN_FLIGHT := YES
else ifeq ($(CLEAN_BUILD), NEVER)
CLEAN_GROUND := NO
CLEAN_FLIGHT := NO
else
CLEAN_GROUND := YES
CLEAN_FLIGHT := YES
endif

# Set up targets (PPM target seems to be broken at the moment)
FW_TARGETS_COMMON := $(addprefix fw_, ahrs pipxtreme)
FW_TARGETS_PWM    := $(addprefix fw_, coptercontrol openpilot)
FW_TARGETS_DSM2   := $(addprefix fw_, coptercontrol openpilot)
FW_TARGETS_SBUS   := $(addprefix fw_, coptercontrol)
FW_TARGETS_PPM    := $(addprefix fw_, openpilot)
FW_TARGETS_TOOLS  := $(addprefix fw_, coptercontrol)
BL_TARGETS        := $(addprefix bl_, coptercontrol openpilot ahrs pipxtreme)
BU_TARGETS        := $(addprefix bu_, coptercontrol openpilot ahrs pipxtreme)

help:
	@echo
	@echo "   This Makefile is known to work on Linux and Mac in a standard shell environment."
	@echo "   It also works on Windows by following the instructions in ../make/winx86/README.txt."
	@echo
	@echo "   Here is a summary of the available targets:"
	@echo
	@echo "   [Packaging]"
	@echo "     package        - Build and package the OpenPilot distributable"
	@echo "     package_flight - Build and package the OpenPilot flight firmware only"
	@echo
	@echo "   Notes:"
	@echo "     - package will be placed in $(PACKAGE_DIR)"
	@echo
	@echo "     - the build directory will be removed first on every run unless one"
	@echo "       of CLEAN_BUILD=NO or CLEAN_BUILD=NEVER options is defined."
	@echo
	@echo "       CLEAN_BUILD=NO means no clean before build except for multi-input"
	@echo "       firmware binaries like CopterControl or OpenPilot. This usually is"
	@echo "       safe."
	@echo
	@echo "       CLEAN_BUILD=NEVER means no clean will be done at all. This will,"
	@echo "       probably, give invalid multi-input firmware and is recommended"
	@echo "       for package testing only. Do not use for release builds."
	@echo

# Clean and build uavobjects since all parts depend on them
uavobjects: all_clean
	$(V1) $(MAKE) -C $(ROOT_DIR) $@

all_clean:
ifneq ($(CLEAN_GROUND), NO)
	$(V1) $(MAKE) -C $(ROOT_DIR) $@
endif

# Install template:
#   $1 = target
#   $2 = dependencies
#   $3 = install directory (must be defined)
#   $4 = installed file name prefix (optional)
#   $5 = installed file name suffix (optional)
#   $6 = extra make options (for instance, USE_SPEKTRUM=YES)
#   $7 = optional 'clean' string to clean target before rebuild
#   $8 = list of targets to install (without _install suffix)
#   $9 = inner make target (usually install, but can be other to just build)
define INSTALL_TEMPLATE
$(1): $(2)
ifeq ($(7),clean)
ifneq ($$(CLEAN_FLIGHT), NO)
	$$(V1) +$(MAKE) -C $(ROOT_DIR) $(6) $(addsuffix _$(7), $(8))
endif
endif
	$$(V1) +$(MAKE) -C $(ROOT_DIR) INSTALL_DIR=$(3) INSTALL_PFX=$(4) INSTALL_SFX=$(5) $(6) $(addsuffix _$(9), $(8))
.PHONY: $(1)
endef

# Firmware for different input drivers
$(eval $(call INSTALL_TEMPLATE,fw_common,uavobjects,$(FW_DIR),,-$(PACKAGE_LBL),,,$(FW_TARGETS_COMMON),install))
$(eval $(call INSTALL_TEMPLATE,fw_pwm,uavobjects,$(FW_DIR),,-pwm-$(PACKAGE_LBL),,clean,$(FW_TARGETS_PWM),install))
$(eval $(call INSTALL_TEMPLATE,fw_dsm2,uavobjects,$(FW_DIR),,-dsm2sat-$(PACKAGE_LBL),USE_SPEKTRUM=YES,clean,$(FW_TARGETS_DSM2),install))
$(eval $(call INSTALL_TEMPLATE,fw_sbus,uavobjects,$(FW_DIR),,-sbus-$(PACKAGE_LBL),USE_SBUS=YES USE_TELEMETRY=3,clean,$(FW_TARGETS_SBUS),install))
$(eval $(call INSTALL_TEMPLATE,fw_ppm,uavobjects,$(FW_DIR),,-ppm-$(PACKAGE_LBL),USE_PPM=YES,clean,$(FW_TARGETS_PPM),install))

# Bootloaders (change 'install' to 'bin' if you don't want to install bootloaders)
$(eval $(call INSTALL_TEMPLATE,all_bl,uavobjects,$(BL_DIR),,-$(PACKAGE_LBL),,,$(BL_TARGETS),install))

# Bootloader updaters
$(eval $(call INSTALL_TEMPLATE,all_bu,all_bl,$(BU_DIR),,-$(PACKAGE_LBL),,,$(BU_TARGETS),install))

# CopterControl flash eraser tool
$(eval $(call INSTALL_TEMPLATE,fw_tools,uavobjects,$(FE_DIR),,-flash-erase-$(PACKAGE_LBL),ERASE_FLASH=YES,clean,$(FW_TARGETS_TOOLS),install))

# Order-only dependencies
# They are bit complicated to support parallel (-j) builds and to create
# the pwm/ppm/spektrum and CC flash eraser targets in some fixed order

fw_pwm:     |		# default dependencies, will be built first

fw_dsm2:    | fw_pwm	# ordered build

fw_sbus:    | fw_dsm2	# ordered build

fw_ppm:     | fw_sbus   # ordered build

fw_tools:   | fw_ppm    # ordered build

package_fw: | fw_common fw_pwm fw_dsm2 fw_sbus fw_ppm

package_bu: | all_bu

package_flight: | package_fw package_bu fw_tools

package_ground: | ground_package

package: | package_flight package_ground

.PHONY: help uavobjects all_clean package package_flight package_fw package_bu package_ground

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
