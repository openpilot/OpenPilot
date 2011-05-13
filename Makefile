# Set up a default goal
.DEFAULT_GOAL := help

# Set up some macros for common directories within the tree
ROOT_DIR=$(CURDIR)
TOOLS_DIR=$(ROOT_DIR)/tools
BUILD_DIR=$(ROOT_DIR)/build
DL_DIR=$(ROOT_DIR)/downloads

# Clean out undesirable variables from the environment and command-line
# to remove the chance that they will cause problems with our build
define SANITIZE_VAR
$(if $(filter-out undefined,$(origin $(1))),
  $(info *NOTE*      Sanitized $(2) variable '$(1)' from $(origin $(1)))
  MAKEOVERRIDES = $(filter-out $(1)=%,$(MAKEOVERRIDES))
  override $(1) :=
  unexport $(1)
)
endef

# These specific variables can influence gcc in unexpected (and undesirable) ways
SANITIZE_GCC_VARS := TMPDIR GCC_EXEC_PREFIX COMPILER_PATH LIBRARY_PATH
SANITIZE_GCC_VARS += CFLAGS CPATH C_INCLUDE_PATH CPLUS_INCLUDE_PATH OBJC_INCLUDE_PATH DEPENDENCIES_OUTPUT
$(foreach var, $(SANITIZE_GCC_VARS), $(eval $(call SANITIZE_VAR,$(var),disallowed)))

# These specific variables used to be valid but now they make no sense
SANITIZE_DEPRECATED_VARS := USE_BOOTLOADER
$(foreach var, $(SANITIZE_DEPRECATED_VARS), $(eval $(call SANITIZE_VAR,$(var),deprecated)))

# We almost need to consider autoconf/automake instead of this
# I don't know if windows supports uname :-(
QT_SPEC=win32-g++
UAVOBJGENERATOR="$(BUILD_DIR)/ground/uavobjgenerator/debug/uavobjgenerator.exe"
UNAME := $(shell uname)
ifeq ($(UNAME), Linux)
  QT_SPEC=linux-g++
  UAVOBJGENERATOR="$(BUILD_DIR)/ground/uavobjgenerator/uavobjgenerator"
endif
ifeq ($(UNAME), Darwin)
  QT_SPEC=macx-g++
  UAVOBJGENERATOR="$(BUILD_DIR)/ground/uavobjgenerator/uavobjgenerator"
endif

# OpenPilot GCS build configuration (debug | release)
GCS_BUILD_CONF ?= debug

# Set up misc host tools
RM=rm

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

.PHONY: help
help:
	@echo
	@echo "   This Makefile is known to work on Linux and Mac in a standard shell environment."
	@echo "   It also works on Windows by following the instructions in make/winx86/README.txt."
	@echo
	@echo "   Here is a summary of the available targets:"
	@echo
	@echo "   [Tool Installers]"
	@echo "     qt_sdk_install       - Install the QT v4.6.2 tools"
	@echo "     arm_sdk_install      - Install the Code Sourcery ARM gcc toolchain"
	@echo "     openocd_install      - Install the OpenOCD JTAG daemon"
	@echo
	@echo "   [Big Hammer]"
	@echo "     all                  - Generate UAVObjects, build openpilot firmware and gcs"
	@echo "     all_flight           - Build all firmware, bootloaders and bootloader updaters"
	@echo "     all_fw               - Build only firmware for all boards"
	@echo "     all_bl               - Build only bootloaders for all boards"
	@echo "     all_blupd            - Build only bootloader updaters for all boards"
	@echo
	@echo "     all_clean            - Remove your build directory ($(BUILD_DIR))"
	@echo "     all_flight_clean     - Remove all firmware, bootloaders and bootloader updaters"
	@echo "     all_fw_clean         - Remove firmware for all boards"
	@echo "     all_bl_clean         - Remove bootlaoders for all boards"
	@echo "     all_blupd_clean      - Remove bootloader updaters for all boards"
	@echo
	@echo "   [Firmware]"
	@echo "     <board>              - Build firmware for <board>"
	@echo "                            supported boards are ($(FW_TARGETS))"
	@echo "     <board>_clean        - Remove firmware for <board>"
	@echo "     <board>_program      - Use OpenOCD + JTAG to write firmware to <board>"
	@echo
	@echo "   [Bootloader]"
	@echo "     bl_<board>           - Build bootloader for <board>"
	@echo "                            supported boards are ($(BL_TARGETS))"
	@echo "     bl_<board>_clean     - Remove bootloader for <board>"
	@echo "     bl_<board>_program   - Use OpenOCD + JTAG to write bootloader to <board>"
	@echo
	@echo "   [Bootloader Updater]"
	@echo "     blupd_<board>        - Build bootloader updater for <board>"
	@echo "                            supported boards are ($(BLUPD_TARGETS))"
	@echo "     blupd_<board>_clean  - Remove bootloader updater for <board>"
	@echo
	@echo "   [Simulation]"
	@echo "     sim_posix            - Build OpenPilot simulation firmware for"
	@echo "                            a POSIX compatible system (Linux, Mac OS X, ...)"
	@echo "     sim_posix_clean      - Delete all build output for the POSIX simulation"
	@echo "     sim_win32            - Build OpenPilot simulation firmware for"
	@echo "                            Windows using mingw and msys"
	@echo "     sim_win32_clean      - Delete all build output for the win32 simulation"
	@echo
	@echo "   [GCS]"
	@echo "     gcs                  - Build the Ground Control System (GCS) application"
	@echo "     gcs_clean            - Remove the Ground Control System (GCS) application"
	@echo
	@echo "   [UAVObjects]"
	@echo "     uavobjects           - Generate source files from the UAVObject definition XML files"
	@echo "     uavobjects_test      - parse xml-files - check for valid, duplicate ObjId's, ... "
	@echo "     uavobjects_<group>   - Generate source files from a subset of the UAVObject definition XML files"
	@echo "                            supported groups are ($(UAVOBJ_TARGETS))"
	@echo
	@echo "   Note: All tools will be installed into $(TOOLS_DIR)"
	@echo "         All build output will be placed in $(BUILD_DIR)"
	@echo

.PHONY: all
all: uavobjects all_ground all_flight

.PHONY: all_clean
all_clean:
	[ ! -d "$(BUILD_DIR)" ] || $(RM) -rf "$(BUILD_DIR)"

$(DL_DIR):
	mkdir -p $@

$(TOOLS_DIR):
	mkdir -p $@

$(BUILD_DIR):
	mkdir -p $@

###############################################################
#
# Installers for tools required by the ground and flight builds
#
# NOTE: These are not tied to the default goals
#       and must be invoked manually
#
###############################################################

# Set up QT toolchain
QT_SDK_DIR := $(TOOLS_DIR)/qtsdk-2010.02

.PHONY: qt_sdk_install
qt_sdk_install: QT_SDK_URL  := http://get.qt.nokia.com/qtsdk/qt-sdk-linux-x86-opensource-2010.02.bin
qt_sdk_install: QT_SDK_FILE := $(notdir $(QT_SDK_URL))
# order-only prereq on directory existance:
qt_sdk_install : | $(DL_DIR) $(TOOLS_DIR)
qt_sdk_install: qt_sdk_clean
        # download the source only if it's newer than what we already have
	$(V1) wget -N -P "$(DL_DIR)" "$(QT_SDK_URL)"

        #installer is an executable, make it executable and run it
	$(V1) chmod u+x "$(DL_DIR)/$(QT_SDK_FILE)"
	"$(DL_DIR)/$(QT_SDK_FILE)" --installdir "$(QT_SDK_DIR)"

.PHONY: qt_sdk_clean
qt_sdk_clean:
	$(V1) [ ! -d "$(QT_SDK_DIR)" ] || $(RM) -rf $(QT_SDK_DIR)

# Set up ARM (STM32) SDK
ARM_SDK_DIR := $(TOOLS_DIR)/arm-2009q3

.PHONY: arm_sdk_install
arm_sdk_install: ARM_SDK_URL  := http://www.codesourcery.com/sgpp/lite/arm/portal/package5353/public/arm-none-eabi/arm-2009q3-68-arm-none-eabi-i686-pc-linux-gnu.tar.bz2
arm_sdk_install: ARM_SDK_FILE := $(notdir $(ARM_SDK_URL))
# order-only prereq on directory existance:
arm_sdk_install: | $(DL_DIR) $(TOOLS_DIR)
arm_sdk_install: arm_sdk_clean
        # download the source only if it's newer than what we already have
	$(V1) wget -N -P "$(DL_DIR)" "$(ARM_SDK_URL)"

        # binary only release so just extract it
	$(V1) tar -C $(TOOLS_DIR) -xjf "$(DL_DIR)/$(ARM_SDK_FILE)"

.PHONY: arm_sdk_clean
arm_sdk_clean:
	$(V1) [ ! -d "$(ARM_SDK_DIR)" ] || $(RM) -r $(ARM_SDK_DIR)

# Set up openocd tools
OPENOCD_DIR := $(TOOLS_DIR)/openocd

.PHONY: openocd_install
openocd_install: OPENOCD_URL  := http://sourceforge.net/projects/openocd/files/openocd/0.4.0/openocd-0.4.0.tar.bz2/download
openocd_install: OPENOCD_FILE := openocd-0.4.0.tar.bz2
# order-only prereq on directory existance:
openocd_install: | $(DL_DIR) $(TOOLS_DIR)
openocd_install: openocd_clean
        # download the source only if it's newer than what we already have
	$(V1) wget -N -P "$(DL_DIR)" --trust-server-name "$(OPENOCD_URL)"

        # extract the source
	$(V1) [ ! -d "$(DL_DIR)/openocd-build" ] || $(RM) -r "$(DL_DIR)/openocd-build"
	$(V1) mkdir -p "$(DL_DIR)/openocd-build"
	$(V1) tar -C $(DL_DIR)/openocd-build -xjf "$(DL_DIR)/$(OPENOCD_FILE)"

        # build and install
	$(V1) mkdir -p "$(OPENOCD_DIR)"
	$(V1) ( \
	  cd $(DL_DIR)/openocd-build/openocd-0.4.0 ; \
	  ./configure --prefix="$(OPENOCD_DIR)" --enable-ft2232_libftdi ; \
	  $(MAKE) ; \
	  $(MAKE) install ; \
	)

        # delete the extracted source when we're done
	$(V1) [ ! -d "$(DL_DIR)/openocd-build" ] || $(RM) -r "$(DL_DIR)/openocd-build"

.PHONY: openocd_clean
openocd_clean:
	$(V1) [ ! -d "$(OPENOCD_DIR)" ] || $(RM) -r "$(OPENOCD_DIR)"

##############################
#
# Set up paths to tools
#
##############################

ifeq ($(shell [ -d "$(QT_SDK_DIR)" ] && echo "exists"), exists)
  QMAKE=$(QT_SDK_DIR)/qt/bin/qmake
else
  # not installed, hope it's in the path...
  QMAKE=qmake
endif

ifeq ($(shell [ -d "$(ARM_SDK_DIR)" ] && echo "exists"), exists)
  ARM_SDK_PREFIX := $(ARM_SDK_DIR)/bin/arm-none-eabi-
else
  # not installed, hope it's in the path...
  ARM_SDK_PREFIX ?= arm-none-eabi-
endif

ifeq ($(shell [ -d "$(OPENOCD_DIR)" ] && echo "exists"), exists)
  OPENOCD := $(OPENOCD_DIR)/bin/openocd
else
  # not installed, hope it's in the path...
  OPENOCD ?= openocd
endif

##############################
#
# GCS related components
#
##############################

.PHONY: all_ground
all_ground: openpilotgcs

# Convenience target for the GCS
.PHONY: gcs gcs_clean
gcs: openpilotgcs
gcs_clean: openpilotgcs_clean

.PHONY: openpilotgcs
openpilotgcs:  uavobjects_gcs
	$(V1) mkdir -p $(BUILD_DIR)/ground/$@
	$(V1) ( cd $(BUILD_DIR)/ground/$@ ; \
	  $(QMAKE) $(ROOT_DIR)/ground/openpilotgcs/openpilotgcs.pro -spec $(QT_SPEC) -r CONFIG+=$(GCS_BUILD_CONF) ; \
	  $(MAKE) -w ; \
	)

.PHONY: gcs_installer
gcs_installer: openpilotgcs
ifeq ($(QT_SPEC), win32-g++)
ifeq ($(GCS_BUILD_CONF), release)
	$(V1) cd $(BUILD_DIR)/ground/openpilotgcs/packaging/winx86 && $(MAKE) -r --no-print-directory $@
else
	$(error $@ can be generated for release build only (GCS_BUILD_CONF=release))
endif
else
	$(error $@ is currently only available on Windows)
endif

.PHONY: openpilotgcs_clean
openpilotgcs_clean:
	$(V0) @echo " CLEAN     $@"
	$(V1) [ ! -d "$(BUILD_DIR)/ground/openpilotgcs" ] || $(RM) -r "$(BUILD_DIR)/ground/openpilotgcs"

.PHONY: uavobjgenerator
uavobjgenerator:
	$(V1) mkdir -p $(BUILD_DIR)/ground/$@
	$(V1) ( cd $(BUILD_DIR)/ground/$@ ; \
	  $(QMAKE) $(ROOT_DIR)/ground/uavobjgenerator/uavobjgenerator.pro -spec $(QT_SPEC) -r CONFIG+=debug ; \
	  $(MAKE) --no-print-directory -w ; \
	)

UAVOBJ_TARGETS := gcs flight python matlab java
.PHONY:uavobjects
uavobjects:  $(addprefix uavobjects_, $(UAVOBJ_TARGETS))

UAVOBJ_XML_DIR := $(ROOT_DIR)/shared/uavobjectdefinition
UAVOBJ_OUT_DIR := $(BUILD_DIR)/uavobject-synthetics

$(UAVOBJ_OUT_DIR):
	$(V1) mkdir -p $@

uavobjects_%: $(UAVOBJ_OUT_DIR) uavobjgenerator
	$(V1) ( cd $(UAVOBJ_OUT_DIR) ; \
	  $(UAVOBJGENERATOR) -$* $(UAVOBJ_XML_DIR) $(ROOT_DIR) ; \
	)

uavobjects_test: $(UAVOBJ_OUT_DIR) uavobjgenerator
	$(V1) $(UAVOBJGENERATOR) -v -none $(UAVOBJ_XML_DIR) $(ROOT_DIR)

uavobjects_clean:
	$(V0) @echo " CLEAN     $@"
	$(V1) [ ! -d "$(UAVOBJ_OUT_DIR)" ] || $(RM) -r "$(UAVOBJ_OUT_DIR)"

##############################
#
# Flight related components
#
##############################

FW_TARGETS    := openpilot ahrs coptercontrol pipxtreme ins
BL_TARGETS    := $(addprefix bl_, $(FW_TARGETS))
BLUPD_TARGETS := $(addprefix blupd_, $(FW_TARGETS))

# FIXME: The INS build doesn't have a bootloader or bootloader
#        updater yet so we need to filter them out to prevent errors.
BL_TARGETS    := $(filter-out bl_ins, $(BL_TARGETS))
BLUPD_TARGETS := $(filter-out blupd_ins, $(BLUPD_TARGETS))

.PHONY: all_fw all_fw_clean
all_fw:           $(addsuffix _bin,   $(FW_TARGETS))
all_fw_clean:     $(addsuffix _clean, $(FW_TARGETS))

.PHONY: all_bl all_bl_clean
all_bl:           $(addsuffix _bin,   $(BL_TARGETS))
all_bl_clean:     $(addsuffix _clean, $(BL_TARGETS))

.PHONY: all_blupd all_blupd_clean
all_blupd:        $(addsuffix _bin,   $(BLUPD_TARGETS))
all_blupd_clean:  $(addsuffix _clean, $(BLUPD_TARGETS))

.PHONY: all_flight all_flight_clean
all_flight:       all_fw all_bl all_blupd
all_flight_clean: all_fw_clean all_bl_clean all_blupd_clean

.PHONY: openpilot
openpilot: openpilot_bin

openpilot_%: uavobjects_flight
	$(V1) mkdir -p $(BUILD_DIR)/openpilot/dep
	$(V1) cd $(ROOT_DIR)/flight/OpenPilot && \
		$(MAKE) -r --no-print-directory \
		OUTDIR="$(BUILD_DIR)/openpilot" TCHAIN_PREFIX="$(ARM_SDK_PREFIX)" \
		REMOVE_CMD="$(RM)" OOCD_EXE="$(OPENOCD)" $*

.PHONY: openpilot_clean
openpilot_clean:
	$(V0) @echo " CLEAN     $@"
	$(V1) $(RM) -fr $(BUILD_DIR)/openpilot

.PHONY: bl_openpilot
bl_openpilot: bl_openpilot_bin
bl_openpilot_bino: bl_openpilot_bin

bl_openpilot_%:
	$(V1) mkdir -p $(BUILD_DIR)/bl_openpilot/dep
	$(V1) cd $(ROOT_DIR)/flight/Bootloaders/OpenPilot && \
		$(MAKE) -r --no-print-directory \
		OUTDIR="$(BUILD_DIR)/bl_openpilot" TCHAIN_PREFIX="$(ARM_SDK_PREFIX)" \
		REMOVE_CMD="$(RM)" OOCD_EXE="$(OPENOCD)" $*

.PHONY: bl_openpilot_clean
bl_openpilot_clean:
	$(V0) @echo " CLEAN     $@"
	$(V1) $(RM) -fr $(BUILD_DIR)/bl_openpilot

.PHONY: blupd_openpilot
blupd_openpilot: blupd_openpilot_bin

blupd_openpilot_%: bl_openpilot_bino
	$(V1) mkdir -p $(BUILD_DIR)/blupd_openpilot/dep
	$(V1) cd $(ROOT_DIR)/flight/Bootloaders/BootloaderUpdater && \
		$(MAKE) -r --no-print-directory \
		OUTDIR="$(BUILD_DIR)/blupd_openpilot" TCHAIN_PREFIX="$(ARM_SDK_PREFIX)" \
		REMOVE_CMD="$(RM)" OOCD_EXE="$(OPENOCD)" \
		BOARD=STM3210E_OP MODEL=HD MODEL_SUFFIX=_OP \
		BLOBJ=$(BUILD_DIR)/bl_openpilot/OpenPilot_BL.bin.o $*

.PHONY: blupd_openpilot_clean
blupd_openpilot_clean:
	$(V0) @echo " CLEAN     $@"
	$(V1) $(RM) -fr $(BUILD_DIR)/blupd_openpilot

.PHONY: ahrs
ahrs: ahrs_bin

ahrs_%: uavobjects_flight
	$(V1) mkdir -p $(BUILD_DIR)/ahrs/dep
	$(V1) cd $(ROOT_DIR)/flight/AHRS && \
		$(MAKE) -r --no-print-directory \
		OUTDIR="$(BUILD_DIR)/ahrs" TCHAIN_PREFIX="$(ARM_SDK_PREFIX)" \
		REMOVE_CMD="$(RM)" OOCD_EXE="$(OPENOCD)" $*

.PHONY: ahrs_clean
ahrs_clean:
	$(V0) @echo " CLEAN     $@"
	$(V1) $(RM) -fr $(BUILD_DIR)/ahrs

.PHONY: bl_ahrs
bl_ahrs: bl_ahrs_bin
bl_ahrs_bino: bl_ahrs_bin

bl_ahrs_%:
	$(V1) mkdir -p $(BUILD_DIR)/bl_ahrs/dep
	$(V1) cd $(ROOT_DIR)/flight/Bootloaders/AHRS && \
		$(MAKE) -r --no-print-directory \
		OUTDIR="$(BUILD_DIR)/bl_ahrs" TCHAIN_PREFIX="$(ARM_SDK_PREFIX)" \
		REMOVE_CMD="$(RM)" OOCD_EXE="$(OPENOCD)" $*

.PHONY: bl_ahrs_clean
bl_ahrs_clean:
	$(V0) @echo " CLEAN     $@"
	$(V1) $(RM) -fr $(BUILD_DIR)/bl_ahrs

.PHONY: blupd_ahrs
blupd_ahrs: blupd_ahrs_bin

blupd_ahrs_%: bl_ahrs_bino bl_ahrs
	$(V1) mkdir -p $(BUILD_DIR)/blupd_ahrs/dep
	$(V1) cd $(ROOT_DIR)/flight/Bootloaders/BootloaderUpdater && \
		$(MAKE) -r --no-print-directory \
		OUTDIR="$(BUILD_DIR)/blupd_ahrs" TCHAIN_PREFIX="$(ARM_SDK_PREFIX)" \
		REMOVE_CMD="$(RM)" OOCD_EXE="$(OPENOCD)" \
		BOARD=STM32103CB_AHRS MODEL=MD \
		BLOBJ=$(BUILD_DIR)/bl_ahrs/AHRS_BL.bin.o $*

.PHONY: blupd_ahrs_clean
blupd_ahrs_clean:
	$(V0) @echo " CLEAN     $@"
	$(V1) $(RM) -fr $(BUILD_DIR)/blupd_ahrs

.PHONY: coptercontrol
coptercontrol: coptercontrol_bin

coptercontrol_%: uavobjects_flight
	$(V1) mkdir -p $(BUILD_DIR)/coptercontrol/dep
	$(V1) cd $(ROOT_DIR)/flight/CopterControl && \
		$(MAKE) -r --no-print-directory \
		OUTDIR="$(BUILD_DIR)/coptercontrol" TCHAIN_PREFIX="$(ARM_SDK_PREFIX)" \
		REMOVE_CMD="$(RM)" OOCD_EXE="$(OPENOCD)" $*

.PHONY: coptercontrol_clean
coptercontrol_clean:
	$(V0) @echo " CLEAN     $@"
	$(V1) $(RM) -fr $(BUILD_DIR)/coptercontrol

.PHONY: bl_coptercontrol
bl_coptercontrol: bl_coptercontrol_bin
bl_coptercontrol_bino: bl_coptercontrol_bin

bl_coptercontrol_%:
	$(V1) mkdir -p $(BUILD_DIR)/bl_coptercontrol/dep
	$(V1) cd $(ROOT_DIR)/flight/Bootloaders/CopterControl && \
		$(MAKE) -r --no-print-directory \
		OUTDIR="$(BUILD_DIR)/bl_coptercontrol" TCHAIN_PREFIX="$(ARM_SDK_PREFIX)" \
		REMOVE_CMD="$(RM)" OOCD_EXE="$(OPENOCD)" $*

.PHONY: bl_coptercontrol_clean
bl_coptercontrol_clean:
	$(V0) @echo " CLEAN     $@"
	$(V1) $(RM) -fr $(BUILD_DIR)/bl_coptercontrol

.PHONY: blupd_coptercontrol
blupd_coptercontrol: blupd_coptercontrol_bin

blupd_coptercontrol_%: bl_coptercontrol_bino
	$(V1) mkdir -p $(BUILD_DIR)/blupd_coptercontrol/dep
	$(V1) cd $(ROOT_DIR)/flight/Bootloaders/BootloaderUpdater && \
		$(MAKE) -r --no-print-directory \
		OUTDIR="$(BUILD_DIR)/blupd_coptercontrol" TCHAIN_PREFIX="$(ARM_SDK_PREFIX)" \
		REMOVE_CMD="$(RM)" OOCD_EXE="$(OPENOCD)" \
		BOARD=STM32103CB_CC_Rev1 MODEL=MD MODEL_SUFFIX=_CC \
		BLOBJ=$(BUILD_DIR)/bl_coptercontrol/CopterControl_BL.bin.o $*

.PHONY: blupd_coptercontrol_clean
blupd_coptercontrol_clean:
	$(V0) @echo " CLEAN     $@"
	$(V1) $(RM) -fr $(BUILD_DIR)/blupd_coptercontrol

.PHONY: pipxtreme
pipxtreme: pipxtreme_bin

pipxtreme_%: uavobjects_flight
	$(V1) mkdir -p $(BUILD_DIR)/pipxtreme/dep
	$(V1) cd $(ROOT_DIR)/flight/PipXtreme && \
		$(MAKE) -r --no-print-directory \
		OUTDIR="$(BUILD_DIR)/pipxtreme" TCHAIN_PREFIX="$(ARM_SDK_PREFIX)" \
		REMOVE_CMD="$(RM)" OOCD_EXE="$(OPENOCD)" $*

.PHONY: pipxtreme_clean
pipxtreme_clean:
	$(V0) @echo " CLEAN     $@"
	$(V1) $(RM) -fr $(BUILD_DIR)/pipxtreme

.PHONY: bl_pipxtreme
bl_pipxtreme: bl_pipxtreme_bin
bl_pipxtreme_bino: bl_pipxtreme_bin

bl_pipxtreme_%:
	$(V1) mkdir -p $(BUILD_DIR)/bl_pipxtreme/dep
	$(V1) cd $(ROOT_DIR)/flight/Bootloaders/PipXtreme && \
		$(MAKE) -r --no-print-directory \
		OUTDIR="$(BUILD_DIR)/bl_pipxtreme" TCHAIN_PREFIX="$(ARM_SDK_PREFIX)" \
		REMOVE_CMD="$(RM)" OOCD_EXE="$(OPENOCD)" $*

.PHONY: bl_pipxtreme_clean
bl_pipxtreme_clean:
	$(V0) @echo " CLEAN     $@"
	$(V1) $(RM) -fr $(BUILD_DIR)/bl_pipxtreme

.PHONY: blupd_pipxtreme
blupd_pipxtreme: blupd_pipxtreme_bin

blupd_pipxtreme_%: bl_pipxtreme_bino
	$(V1) mkdir -p $(BUILD_DIR)/blupd_pipxtreme/dep
	$(V1) cd $(ROOT_DIR)/flight/Bootloaders/BootloaderUpdater && \
		$(MAKE) -r --no-print-directory \
		OUTDIR="$(BUILD_DIR)/blupd_pipxtreme" TCHAIN_PREFIX="$(ARM_SDK_PREFIX)" \
		REMOVE_CMD="$(RM)" OOCD_EXE="$(OPENOCD)" \
		BOARD=STM32103CB_PIPXTREME MODEL=MD MODEL_SUFFIX=_CC \
		BLOBJ=$(BUILD_DIR)/bl_pipxtreme/PipXtreme_BL.bin.o $*

.PHONY: blupd_pipxtreme_clean
blupd_pipxtreme_clean:
	$(V0) @echo " CLEAN     $@"
	$(V1) $(RM) -fr $(BUILD_DIR)/blupd_pipxtreme


.PHONY: ins
ins: ins_bin

ins_%: uavobjects_flight
	$(V1) mkdir -p $(BUILD_DIR)/ins/dep
	$(V1) cd $(ROOT_DIR)/flight/INS && \
		$(MAKE) -r --no-print-directory \
		OUTDIR="$(BUILD_DIR)/ins" TCHAIN_PREFIX="$(ARM_SDK_PREFIX)" \
		REMOVE_CMD="$(RM)" OOCD_EXE="$(OPENOCD)" $*

.PHONY: ins_clean
ins_clean:
	$(V0) @echo " CLEAN     $@"
	$(V1) $(RM) -fr $(BUILD_DIR)/ins

.PHONY: bl_ins
bl_ins: bl_ins_elf

bl_ins_%:
	$(V1) mkdir -p $(BUILD_DIR)/bl_ins/dep
	$(V1) cd $(ROOT_DIR)/flight/Bootloaders/INS && \
		$(MAKE) -r --no-print-directory \
		OUTDIR="$(BUILD_DIR)/bl_ins" TCHAIN_PREFIX="$(ARM_SDK_PREFIX)" \
		REMOVE_CMD="$(RM)" OOCD_EXE="$(OPENOCD)" $*

.PHONY: bl_ins_clean
bl_ins_clean:
	$(V0) @echo " CLEAN     $@"
	$(V1) $(RM) -fr $(BUILD_DIR)/bl_ins


.PHONY: sim_posix
sim_posix: sim_posix_elf

sim_posix_%: uavobjects_flight
	$(V1) mkdir -p $(BUILD_DIR)/sitl_posix
	$(V1) $(MAKE) --no-print-directory \
		-C $(ROOT_DIR)/flight/OpenPilot --file=$(ROOT_DIR)/flight/OpenPilot/Makefile.posix $*

.PHONY: sim_win32
sim_win32: sim_win32_exe

sim_win32_%: uavobjects_flight
	$(V1) mkdir -p $(BUILD_DIR)/sitl_win32
	$(V1) $(MAKE) --no-print-directory \
		-C $(ROOT_DIR)/flight/OpenPilot --file=$(ROOT_DIR)/flight/OpenPilot/Makefile.win32 $*
