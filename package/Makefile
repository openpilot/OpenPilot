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
# If there is a tag, use it for label instead of date-hash string
PACKAGE_TAG  := $(shell $(VERSION_CMD) --format=\$${TAG})
ifneq ($(PACKAGE_TAG),)
PACKAGE_LBL  := $(shell $(VERSION_CMD) --format=\$${TAG}\$${DIRTY})
else
PACKAGE_LBL  := $(shell $(VERSION_CMD) --format=\$${DATE}-\$${TAG_OR_HASH8}\$${DIRTY})
endif
PACKAGE_DIR  := $(BUILD_DIR)/package-$(PACKAGE_LBL)
FW_DIR       := $(PACKAGE_DIR)/firmware-$(PACKAGE_LBL)
BL_DIR       := $(FW_DIR)/bootloaders
BU_DIR       := $(FW_DIR)/bootloader-updaters

# Clean build options (recommended for package testing only)
ifeq ($(CLEAN_BUILD), NO)
CLEAN_GROUND := NO
CLEAN_FLIGHT := NO
else
CLEAN_GROUND := YES
CLEAN_FLIGHT := YES
endif

# Set up targets
ALL_BOARDS        := coptercontrol pipxtreme revolution revomini osd
ALL_BOARDS_BU     := coptercontrol pipxtreme
FW_TARGETS        := $(addprefix fw_, $(ALL_BOARDS))
FW_TARGETS_TOOLS  := $(addprefix fw_, $(ALL_BOARDS))
BL_TARGETS        := $(addprefix bl_, $(ALL_BOARDS))
BU_TARGETS        := $(addprefix bu_, $(ALL_BOARDS_BU))

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
	@echo "     - the build directory will be removed first on every run unless"
	@echo "       CLEAN_BUILD=NO is defined. It means no clean before build."
	@echo "       This usually is safe."
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

# Firmware
$(eval $(call INSTALL_TEMPLATE,all_fw,uavobjects,$(FW_DIR),,-$(PACKAGE_LBL),,,$(FW_TARGETS),install))

# Bootloaders (change 'install' to 'bin' if you don't want to install bootloaders)
$(eval $(call INSTALL_TEMPLATE,all_bl,uavobjects,$(BL_DIR),,-$(PACKAGE_LBL),,,$(BL_TARGETS),install))

# Bootloader updaters
$(eval $(call INSTALL_TEMPLATE,all_bu,all_bl,$(BU_DIR),,-$(PACKAGE_LBL),,,$(BU_TARGETS),install))

# Order-only dependencies
package_flight: | all_fw all_bu

package_ground: | ground_package

package: | package_flight package_ground

.PHONY: help uavobjects all_clean package_flight package_ground package

# opfw_resource must be generated before the ground package,
# and it depends on flight firmware images
ground_package: | opfw_resource

opfw_resource: | all_fw

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

toprel = $(subst $(realpath $(ROOT_DIR))/,,$(abspath $(1)))
OPFW_FILES := $(foreach fw_targ, $(FW_TARGETS), $(call toprel, $(BUILD_DIR)/$(fw_targ)/$(fw_targ).opfw))
OPFW_CONTENTS := \
<!DOCTYPE RCC><RCC version="1.0"> \
 <qresource> \
     $(foreach fw_file, $(OPFW_FILES), <file>$(fw_file)</file>) \
 </qresource> \
</RCC>

.PHONY: opfw_resource
opfw_resource:
	@echo Generating OPFW resource file $(call toprel, $(BUILD_DIR)/ground/$@)
	$(V1) mkdir -p $(BUILD_DIR)/ground/$@
	$(V1) mkdir -p $(BUILD_DIR)/ground/$@/build
	$(V1) echo '$(OPFW_CONTENTS)' > $(BUILD_DIR)/ground/$@/opfw_resource.qrc
	$(V1) $(foreach fw_targ, $(FW_TARGETS), mkdir -p $(BUILD_DIR)/ground/$@/build/$(fw_targ);)
	$(V1)$(foreach fw_targ, $(FW_TARGETS), cp $(BUILD_DIR)/$(fw_targ)/$(fw_targ).opfw $(BUILD_DIR)/ground/$@/build/$(fw_targ)/;)

include $(WHEREAMI)/Makefile.$(PLATFORM)
