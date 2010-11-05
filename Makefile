# Set up some macros for common directories within the tree
ROOT_DIR=$(CURDIR)
TOOLS_DIR=$(ROOT_DIR)/tools
BUILD_DIR=$(ROOT_DIR)/build
DL_DIR=$(ROOT_DIR)/downloads

# We almost need to consider autoconf/automake instead of this
# I don't know if windows supports uname :-(
QT_SPEC=win32-g++
UNAME := $(shell uname)
ifeq ($(UNAME), Linux)
  QT_SPEC=linux-g++
endif
ifeq ($(UNAME), Darwin)
  QT_SPEC=macx-g++
endif

# Set up misc host tools
RM=rm

.PHONY: areyousureyoushouldberunningthis
areyousureyoushouldberunningthis:
	@echo
	@echo "   This Makefile is somewhat experimental and will probably only work on Linux right now."
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
	@echo
	@echo "   [Simulation]"
	@echo "     sim_posix         - Build OpenPilot simulation firmware for"
	@echo "                         a POSIX compatible system (Linux, Mac OS X, ...)"
	@echo "     sim_posix_clean   - Delete all build output for the POSIX simulation"
	@echo
	@echo "   [GCS and UAVObjects]"
	@echo "     gcs               - Build the Ground Control System application"
	@echo "     uavobjects        - Generate the gcs and openpilot source files from the UAVObject definition XML files"
	@echo
	@echo "   Note: All tools will be installed into $(TOOLS_DIR)"
	@echo "         All build output will be placed in $(BUILD_DIR)"
	@echo

.PHONY: all
all: uavobjects all_ground all_flight

.PHONY: all_clean
all_clean:
	[ ! -d "$(BUILD_DIR)" ] || $(RM) -r "$(BUILD_DIR)"

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
qt_sdk_install: qt_sdk_clean $(TOOLS_DIR)
	# download the source only if it's newer than what we already have
	wget -N -P "$(DL_DIR)" "$(QT_SDK_URL)"

	#installer is an executable, make it executable and run it
	chmod u+x "$(DL_DIR)/$(QT_SDK_FILE)"
	"$(DL_DIR)/$(QT_SDK_FILE)" --installdir "$(QT_SDK_DIR)"

.PHONY: qt_sdk_clean
qt_sdk_clean:
	[ ! -d "$(QT_SDK_DIR)" ] || $(RM) -rf $(QT_SDK_DIR)

# Set up ARM (STM32) SDK
ARM_SDK_DIR := $(TOOLS_DIR)/arm-2009q3

.PHONY: arm_sdk_install
arm_sdk_install: ARM_SDK_URL  := http://www.codesourcery.com/sgpp/lite/arm/portal/package5353/public/arm-none-eabi/arm-2009q3-68-arm-none-eabi-i686-pc-linux-gnu.tar.bz2
arm_sdk_install: ARM_SDK_FILE := $(notdir $(ARM_SDK_URL))
arm_sdk_install: arm_sdk_clean $(TOOLS_DIR)
	# download the source only if it's newer than what we already have
	wget -N -P "$(DL_DIR)" "$(ARM_SDK_URL)"

	# binary only release so just extract it
	tar -C $(TOOLS_DIR) -xjf "$(DL_DIR)/$(ARM_SDK_FILE)"

.PHONY: arm_sdk_clean
arm_sdk_clean:
	[ ! -d "$(ARM_SDK_DIR)" ] || $(RM) -r $(ARM_SDK_DIR)

# Set up openocd tools
OPENOCD_DIR := $(TOOLS_DIR)/openocd

.PHONY: openocd_install
openocd_install: OPENOCD_URL  := http://sourceforge.net/projects/openocd/files/openocd/0.4.0/openocd-0.4.0.tar.bz2/download
openocd_install: OPENOCD_FILE := openocd-0.4.0.tar.bz2
openocd_install: openocd_clean $(TOOLS_DIR)
	# download the source only if it's newer than what we already have
	wget -N -P "$(DL_DIR)" "$(OPENOCD_URL)"

	# extract the source
	[ ! -d "$(DL_DIR)/openocd-build" ] || $(RM) -r "$(DL_DIR)/openocd-build"
	mkdir -p "$(DL_DIR)/openocd-build"
	tar -C $(DL_DIR)/openocd-build -xjf "$(DL_DIR)/$(OPENOCD_FILE)"

	# build and install
	mkdir -p "$(OPENOCD_DIR)"
	( \
	  cd $(DL_DIR)/openocd-build/openocd-0.4.0 ; \
	  ./configure --prefix="$(OPENOCD_DIR)" --enable-ft2232_libftdi ; \
	  $(MAKE) ; \
	  $(MAKE) install ; \
	)

	# delete the extracted source when we're done
	[ ! -d "$(DL_DIR)/openocd-build" ] || $(RM) -r "$(DL_DIR)/openocd-build"

.PHONY: openocd_clean
openocd_clean:
	[ ! -d "$(OPENOCD_DIR)" ] || $(RM) -r "$(OPENOCD_DIR)"

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
all_ground: uavobjgenerator openpilotgcs

# Convenience target for the GCS
.PHONY: gcs
gcs: openpilotgcs

# Note: openpilotgcs should depend on uavobjects directly since it uses
#       the generated uavobject files.  This is commented out since the
#       uavobjgenerator tool always regenerates its output files
#       triggering unnecessary rebuilds of the elf file.

.PHONY: openpilotgcs
openpilotgcs:  #uavobjects
	mkdir -p $(BUILD_DIR)/$@
	( cd $(BUILD_DIR)/$@ ; \
	  $(QMAKE) $(ROOT_DIR)/ground/openpilotgcs.pro -spec $(QT_SPEC) -r CONFIG+=debug ; \
	  $(MAKE) -w ; \
	)

.PHONY: uavobjgenerator
uavobjgenerator:
	mkdir -p $(BUILD_DIR)/$@
	( cd $(BUILD_DIR)/$@ ; \
	  $(QMAKE) $(ROOT_DIR)/ground/src/libs/uavobjgenerator/uavobjgenerator.pro -spec $(QT_SPEC) -r CONFIG+=debug ; \
	  $(MAKE) -w ; \
	)

.PHONY: uavobjects
uavobjects: uavobjgenerator
	mkdir -p $(BUILD_DIR)/$@
	"$(BUILD_DIR)/uavobjgenerator/uavobjgenerator" "$(ROOT_DIR)/"

##############################
#
# Flight related components
#
##############################

.PHONY: all_flight
all_flight: openpilot_elf ahrs_elf

.PHONY: openpilot
openpilot: openpilot_elf

# Note: openpilot_* should depend on uavobjects directly since it uses
#       the generated uavobject files.  This is commented out since the
#       uavobjgenerator tool always regenerates its output files
#       triggering unnecessary rebuilds of the elf file.

openpilot_%: #uavobjects
	mkdir -p $(BUILD_DIR)/openpilot
	$(MAKE) OUTDIR="$(BUILD_DIR)/openpilot" TCHAIN_PREFIX="$(ARM_SDK_PREFIX)" REMOVE_CMD="$(RM)" OOCD_EXE="$(OPENOCD)" -C $(ROOT_DIR)/flight/OpenPilot $*

.PHONY: ahrs
ahrs: ahrs_elf

ahrs_%:
	mkdir -p $(BUILD_DIR)/ahrs
	$(MAKE) OUTDIR="$(BUILD_DIR)/ahrs" TCHAIN_PREFIX="$(ARM_SDK_PREFIX)" REMOVE_CMD="$(RM)" OOCD_EXE="$(OPENOCD)" -C $(ROOT_DIR)/flight/AHRS $*

.PHONY: sim_posix
sim_posix: sim_posix_elf

# Note: sim_* should depend on uavobjects directly - same reasons as above.

sim_posix_%: #uavobjects
	mkdir -p $(BUILD_DIR)/simulation
	$(MAKE) OUTDIR="$(BUILD_DIR)/simulation" -C $(ROOT_DIR)/flight/OpenPilot --file=$(ROOT_DIR)/flight/OpenPilot/Makefile.posix $*

