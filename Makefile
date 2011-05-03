# Set up some macros for common directories within the tree
ROOT_DIR=$(CURDIR)
TOOLS_DIR=$(ROOT_DIR)/tools
BUILD_DIR=$(ROOT_DIR)/build
DL_DIR=$(ROOT_DIR)/downloads

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

.PHONY: areyousureyoushouldberunningthis
areyousureyoushouldberunningthis:
	@echo
	@echo "   This Makefile will probably only work on Linux and Mac right now."
	@echo "   If you're sure you want to be using this, you may wish to try the following targets:"
	@echo
	@echo "   [Tool Installers]"
	@echo "     qt_sdk_install    - Install the QT v4.6.2 tools"
	@echo "     arm_sdk_install   - Install the Code Sourcery ARM gcc toolchain"
	@echo "     openocd_install   - Install the OpenOCD JTAG daemon"
	@echo
	@echo "   [Big Hammer]"
	@echo "     all               - Generate UAVObjects, build openpilot firmware and gcs"
	@echo "     all_clean         - Remove your build directory ($(BUILD_DIR))"
	@echo
	@echo "   [Firmware]"
	@echo "     openpilot         - Build firmware for the OpenPilot board"
	@echo "     openpilot_clean   - Delete all build output for the OpenPilot firmware"
	@echo "     openpilot_program - Program the firmware onto the OpenPilot board"
	@echo "     ahrs              - Build firmware for the AHRS board"
	@echo "     ahrs_clean        - Delete all build output for the AHRS firmware"
	@echo "     ahrs_program      - Program the firmware onto the AHRS board"
	@echo "     coptercontrol     - Build firmware for the CopterControl board"
	@echo
	@echo "       NOTE: To build firmware to be chain loaded from a bootloader, use"
	@echo "                 make openpilot USE_BOOTLOADER=YES"
	@echo "             Don't forget to do a clean between builds with/without bootloader"
	@echo
	@echo "   [Simulation]"
	@echo "     sim_posix         - Build OpenPilot simulation firmware for"
	@echo "                         a POSIX compatible system (Linux, Mac OS X, ...)"
	@echo "     sim_posix_clean   - Delete all build output for the POSIX simulation"
	@echo "     sim_win32         - Build OpenPilot simulation firmware for"
	@echo "                         Windows using mingw and msys"
	@echo "     sim_win32_clean   - Delete all build output for the win32 simulation"
	@echo
	@echo "   [GCS]"
	@echo "     gcs               - Build the Ground Control System application"
	@echo
	@echo "   [UAVObjects]"
	@echo "     uavobjects        - Generate source files from the UAVObject definition XML files"
	@echo "     uavobjects_test   - parse xml-files - check for valid, duplicate ObjId's, ... "
	@echo "     uavobjects_flight - Generate flight source files from the UAVObject definition XML files"
	@echo "     uavobjects_gcs    - Generate groundstation source files from the UAVObject definition XML files"
	@echo "     uavobjects_python - Generate python source files from the UAVObject definition XML files"
	@echo "     uavobjects_matlab - Generate matlab source files from the UAVObject definition XML files"
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
.PHONY: gcs
gcs: openpilotgcs

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

.PHONY: uavobjgenerator
uavobjgenerator:
	$(V1) mkdir -p $(BUILD_DIR)/ground/$@
	$(V1) ( cd $(BUILD_DIR)/ground/$@ ; \
	  $(QMAKE) $(ROOT_DIR)/ground/uavobjgenerator/uavobjgenerator.pro -spec $(QT_SPEC) -r CONFIG+=debug ; \
	  $(MAKE) --no-print-directory -w ; \
	)

.PHONY:uavobjects
uavobjects:  uavobjects_gcs uavobjects_flight uavobjects_python uavobjects_matlab uavobjects_java

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
	$(V1) [ ! -d "$(UAVOBJ_OUT_DIR)" ] || $(RM) -r "$(UAVOBJ_OUT_DIR)"

##############################
#
# Flight related components
#
##############################

FW_TARGETS := openpilot ahrs coptercontrol pipxtreme ins
BL_TARGETS := $(addprefix bl_, $(FW_TARGETS))

.PHONY: all_fw all_fw_clean
all_fw:           $(addsuffix _bin,   $(FW_TARGETS))
all_fw_clean:     $(addsuffix _clean, $(FW_TARGETS))

.PHONY: all_bw all_bw_clean
all_bl:           $(addsuffix _elf,   $(BL_TARGETS))
all_bl_clean:     $(addsuffix _clean, $(BL_TARGETS))

.PHONY: all_flight all_flight_clean
all_flight:       all_fw all_bl
all_flight_clean: all_fw_clean all_bl_clean

.PHONY: openpilot
openpilot: openpilot_bin

openpilot_%: uavobjects_flight
	$(V1) mkdir -p $(BUILD_DIR)/openpilot/dep
	$(V1) cd $(ROOT_DIR)/flight/OpenPilot && $(MAKE) -r --no-print-directory OUTDIR="$(BUILD_DIR)/openpilot" TCHAIN_PREFIX="$(ARM_SDK_PREFIX)" REMOVE_CMD="$(RM)" OOCD_EXE="$(OPENOCD)" $*

.PHONY: openpilot_clean
openpilot_clean:
	$(V0) @echo " CLEAN     $@"
	$(V1) $(RM) -fr $(BUILD_DIR)/openpilot

.PHONY: bl_openpilot
bl_openpilot: bl_openpilot_elf

bl_openpilot_%:
	$(V1) mkdir -p $(BUILD_DIR)/bl_openpilot/dep
	$(V1) cd $(ROOT_DIR)/flight/Bootloaders/OpenPilot && $(MAKE) -r --no-print-directory OUTDIR="$(BUILD_DIR)/bl_openpilot" TCHAIN_PREFIX="$(ARM_SDK_PREFIX)" REMOVE_CMD="$(RM)" OOCD_EXE="$(OPENOCD)" $*

.PHONY: bl_openpilot_clean
bl_openpilot_clean:
	$(V0) @echo " CLEAN     $@"
	$(V1) $(RM) -fr $(BUILD_DIR)/bl_openpilot

.PHONY: ahrs
ahrs: ahrs_bin

ahrs_%: uavobjects_flight
	$(V1) mkdir -p $(BUILD_DIR)/ahrs/dep
	$(V1) cd $(ROOT_DIR)/flight/AHRS && $(MAKE) -r --no-print-directory OUTDIR="$(BUILD_DIR)/ahrs" TCHAIN_PREFIX="$(ARM_SDK_PREFIX)" REMOVE_CMD="$(RM)" OOCD_EXE="$(OPENOCD)" $*

.PHONY: ahrs_clean
ahrs_clean:
	$(V0) @echo " CLEAN     $@"
	$(V1) $(RM) -fr $(BUILD_DIR)/ahrs

.PHONY: bl_ahrs
bl_ahrs: bl_ahrs_elf

bl_ahrs_%:
	$(V1) mkdir -p $(BUILD_DIR)/bl_ahrs/dep
	$(V1) cd $(ROOT_DIR)/flight/Bootloaders/AHRS && $(MAKE) -r --no-print-directory OUTDIR="$(BUILD_DIR)/bl_ahrs" TCHAIN_PREFIX="$(ARM_SDK_PREFIX)" REMOVE_CMD="$(RM)" OOCD_EXE="$(OPENOCD)" $*

.PHONY: bl_ahrs_clean
bl_ahrs_clean:
	$(V0) @echo " CLEAN     $@"
	$(V1) $(RM) -fr $(BUILD_DIR)/bl_ahrs

.PHONY: coptercontrol
coptercontrol: coptercontrol_bin

coptercontrol_%: uavobjects_flight
	$(V1) mkdir -p $(BUILD_DIR)/coptercontrol/dep
	$(V1) cd $(ROOT_DIR)/flight/CopterControl && $(MAKE) -r --no-print-directory OUTDIR="$(BUILD_DIR)/coptercontrol" TCHAIN_PREFIX="$(ARM_SDK_PREFIX)" REMOVE_CMD="$(RM)" OOCD_EXE="$(OPENOCD)" $*

.PHONY: coptercontrol_clean
coptercontrol_clean:
	$(V0) @echo " CLEAN     $@"
	$(V1) $(RM) -fr $(BUILD_DIR)/coptercontrol

.PHONY: bl_coptercontrol
bl_coptercontrol: bl_coptercontrol_elf

bl_coptercontrol_%:
	$(V1) mkdir -p $(BUILD_DIR)/bl_coptercontrol/dep
	$(V1) cd $(ROOT_DIR)/flight/Bootloaders/CopterControl && $(MAKE) -r --no-print-directory OUTDIR="$(BUILD_DIR)/bl_coptercontrol" TCHAIN_PREFIX="$(ARM_SDK_PREFIX)" REMOVE_CMD="$(RM)" OOCD_EXE="$(OPENOCD)" $*

.PHONY: bl_coptercontrol_clean
bl_coptercontrol_clean:
	$(V0) @echo " CLEAN     $@"
	$(V1) $(RM) -fr $(BUILD_DIR)/bl_coptercontrol

.PHONY: pipxtreme
pipxtreme: pipxtreme_bin

pipxtreme_%: uavobjects_flight
	$(V1) mkdir -p $(BUILD_DIR)/pipxtreme/dep
	$(V1) cd $(ROOT_DIR)/flight/PipXtreme && $(MAKE) -r --no-print-directory OUTDIR="$(BUILD_DIR)/pipxtreme" TCHAIN_PREFIX="$(ARM_SDK_PREFIX)" REMOVE_CMD="$(RM)" OOCD_EXE="$(OPENOCD)" $*

.PHONY: pipxtreme_clean
pipxtreme_clean:
	$(V0) @echo " CLEAN     $@"
	$(V1) $(RM) -fr $(BUILD_DIR)/pipxtreme

.PHONY: bl_pipxtreme
bl_pipxtreme: bl_pipxtreme_elf

bl_pipxtreme_%:
	$(V1) mkdir -p $(BUILD_DIR)/bl_pipxtreme/dep
	$(V1) cd $(ROOT_DIR)/flight/Bootloaders/PipXtreme && $(MAKE) -r --no-print-directory OUTDIR="$(BUILD_DIR)/bl_pipxtreme" TCHAIN_PREFIX="$(ARM_SDK_PREFIX)" REMOVE_CMD="$(RM)" OOCD_EXE="$(OPENOCD)" $*

.PHONY: bl_pipxtreme_clean
bl_pipxtreme_clean:
	$(V0) @echo " CLEAN     $@"
	$(V1) $(RM) -fr $(BUILD_DIR)/bl_pipxtreme

.PHONY: ins
ins: ins_bin

ins_%: uavobjects_flight
	$(V1) mkdir -p $(BUILD_DIR)/ins/dep
	$(V1) cd $(ROOT_DIR)/flight/INS && $(MAKE) -r --no-print-directory OUTDIR="$(BUILD_DIR)/ins" TCHAIN_PREFIX="$(ARM_SDK_PREFIX)" REMOVE_CMD="$(RM)" OOCD_EXE="$(OPENOCD)" $*

.PHONY: ins_clean
ins_clean:
	$(V0) @echo " CLEAN     $@"
	$(V1) $(RM) -fr $(BUILD_DIR)/ins

.PHONY: bl_ins
bl_ins: bl_ins_elf

bl_ins_%:
	$(V1) mkdir -p $(BUILD_DIR)/bl_ins/dep
	$(V1) cd $(ROOT_DIR)/flight/Bootloaders/INS && $(MAKE) -r --no-print-directory OUTDIR="$(BUILD_DIR)/bl_ins" TCHAIN_PREFIX="$(ARM_SDK_PREFIX)" REMOVE_CMD="$(RM)" OOCD_EXE="$(OPENOCD)" $*

.PHONY: bl_ins_clean
bl_ins_clean:
	$(V0) @echo " CLEAN     $@"
	$(V1) $(RM) -fr $(BUILD_DIR)/bl_ins


.PHONY: sim_posix
sim_posix: sim_posix_elf

sim_posix_%: uavobjects_flight
	$(V1) mkdir -p $(BUILD_DIR)/sitl_posix
	$(V1) $(MAKE) --no-print-directory -C $(ROOT_DIR)/flight/OpenPilot --file=$(ROOT_DIR)/flight/OpenPilot/Makefile.posix $*

.PHONY: sim_win32
sim_win32: sim_win32_exe

sim_win32_%: uavobjects_flight
	$(V1) mkdir -p $(BUILD_DIR)/sitl_win32
	$(V1) $(MAKE) --no-print-directory -C $(ROOT_DIR)/flight/OpenPilot --file=$(ROOT_DIR)/flight/OpenPilot/Makefile.win32 $*
