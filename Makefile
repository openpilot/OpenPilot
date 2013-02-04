# Set up a default goal
.DEFAULT_GOAL := help

# Set up some macros for common directories within the tree
ROOT_DIR=$(CURDIR)
TOOLS_DIR=$(ROOT_DIR)/tools
BUILD_DIR=$(ROOT_DIR)/build
DL_DIR=$(ROOT_DIR)/downloads

# Function for converting an absolute path to one relative
# to the top of the source tree.
toprel = $(subst $(realpath $(ROOT_DIR))/,,$(abspath $(1)))

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
ARCH := $(shell uname -m)
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
	@echo "     qt_sdk_install       - Install the QT v4.7.3 tools"
	@echo "     arm_sdk_install      - Install the GNU ARM gcc toolchain"
	@echo "     openocd_install      - Install the OpenOCD JTAG daemon"
	@echo "     stm32flash_install   - Install the stm32flash tool for unbricking boards"
	@echo "     dfuutil_install      - Install the dfu-util tool for unbricking F4-based boards"
	@echo "     android_sdk_install  - Install the Android SDK tools"
	@echo
	@echo "   [Big Hammer]"
	@echo "     all                  - Generate UAVObjects, build openpilot firmware and gcs"
	@echo "     all_flight           - Build all firmware, bootloaders and bootloader updaters"
	@echo "     all_fw               - Build only firmware for all boards"
	@echo "     all_bl               - Build only bootloaders for all boards"
	@echo "     all_bu               - Build only bootloader updaters for all boards"
	@echo
	@echo "     all_clean            - Remove your build directory ($(BUILD_DIR))"
	@echo "     all_flight_clean     - Remove all firmware, bootloaders and bootloader updaters"
	@echo "     all_fw_clean         - Remove firmware for all boards"
	@echo "     all_bl_clean         - Remove bootlaoders for all boards"
	@echo "     all_bu_clean         - Remove bootloader updaters for all boards"
	@echo
	@echo "     all_<board>          - Build all available images for <board>"
	@echo "     all_<board>_clean    - Remove all available images for <board>"
	@echo
	@echo "   [Firmware]"
	@echo "     <board>              - Build firmware for <board>"
	@echo "                            supported boards are ($(ALL_BOARDS))"
	@echo "     fw_<board>           - Build firmware for <board>"
	@echo "                            supported boards are ($(FW_BOARDS))"
	@echo "     fw_<board>_clean     - Remove firmware for <board>"
	@echo "     fw_<board>_program   - Use OpenOCD + JTAG to write firmware to <board>"
	@echo
	@echo "   [Bootloader]"
	@echo "     bl_<board>           - Build bootloader for <board>"
	@echo "                            supported boards are ($(BL_BOARDS))"
	@echo "     bl_<board>_clean     - Remove bootloader for <board>"
	@echo "     bl_<board>_program   - Use OpenOCD + JTAG to write bootloader to <board>"
	@echo
	@echo "   [Bootloader Updater]"
	@echo "     bu_<board>           - Build bootloader updater for <board>"
	@echo "                            supported boards are ($(BU_BOARDS))"
	@echo "     bu_<board>_clean     - Remove bootloader updater for <board>"
	@echo
	@echo "   [Unbrick a board]"
	@echo "     unbrick_<board>      - Use the STM32's built in boot ROM to write a bootloader to <board>"
	@echo "                            supported boards are ($(BL_BOARDS))"
	@echo
	@echo "   [Simulation]"
	@echo "     sim_osx              - Build OpenPilot simulation firmware for OSX"
	@echo "     sim_osx_clean        - Delete all build output for the osx simulation"
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
	@echo "   Hint: Add V=1 to your command line to see verbose build output."
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
QT_SDK_DIR := $(TOOLS_DIR)/qtsdk-v1.2.1

.PHONY: qt_sdk_install
# Choose the appropriate installer based on host architecture
ifneq (,$(filter $(ARCH), x86_64 amd64))
# 64-bit
qt_sdk_install: QT_SDK_FILE := QtSdk-offline-linux-x86_64-v1.2.1.run
qt_sdk_install: QT_SDK_URL := http://www.developer.nokia.com/dp?uri=http://sw.nokia.com/id/14b2039c-0e1f-4774-a4f2-9aa60b6d5313/Qt_SDK_Lin64_offline
else
# 32-bit
qt_sdk_install: QT_SDK_URL  := http://www.developer.nokia.com/dp?uri=http://sw.nokia.com/id/8ea74da4-fec1-4277-8b26-c58cc82e204b/Qt_SDK_Lin32_offline
qt_sdk_install: QT_SDK_FILE := QtSdk-offline-linux-x86-v1.2.1.run
endif
# order-only prereq on directory existance:
qt_sdk_install : | $(DL_DIR) $(TOOLS_DIR)
qt_sdk_install: qt_sdk_clean
        # download the source only if it's newer than what we already have
	$(V1) wget -N --content-disposition -P "$(DL_DIR)" "$(QT_SDK_URL)"
        # tell the user exactly which path they should select in the GUI
	$(V1) echo "*** NOTE NOTE NOTE ***"
	$(V1) echo "*"
	$(V1) echo "*  In the GUI, please use exactly this path as the installation path:"
	$(V1) echo "*        $(QT_SDK_DIR)"
	$(V1) echo "*"
	$(V1) echo "*** NOTE NOTE NOTE ***"

        #installer is an executable, make it executable and run it
	$(V1) chmod u+x "$(DL_DIR)/$(QT_SDK_FILE)"
	$(V1) "$(DL_DIR)/$(QT_SDK_FILE)" -style cleanlooks

.PHONY: qt_sdk_clean
qt_sdk_clean:
	$(V1) [ ! -d "$(QT_SDK_DIR)" ] || $(RM) -rf $(QT_SDK_DIR)

# Set up ARM (STM32) SDK
ARM_SDK_DIR := $(TOOLS_DIR)/gcc-arm-none-eabi-4_6-2012q1

.PHONY: arm_sdk_install
arm_sdk_install: ARM_SDK_URL  := https://launchpad.net/gcc-arm-embedded/4.6/4.6-2012-q1-update/+download/gcc-arm-none-eabi-4_6-2012q1-20120316.tar.bz2
arm_sdk_install: ARM_SDK_FILE := $(notdir $(ARM_SDK_URL))
# order-only prereq on directory existance:
arm_sdk_install: | $(DL_DIR) $(TOOLS_DIR)
arm_sdk_install: arm_sdk_clean
        # download the source only if it's newer than what we already have
	$(V1) wget --no-check-certificate -N -P "$(DL_DIR)" "$(ARM_SDK_URL)"

        # binary only release so just extract it
	$(V1) tar -C $(TOOLS_DIR) -xjf "$(DL_DIR)/$(ARM_SDK_FILE)"

.PHONY: arm_sdk_clean
arm_sdk_clean:
	$(V1) [ ! -d "$(ARM_SDK_DIR)" ] || $(RM) -r $(ARM_SDK_DIR)

# Set up openocd tools
OPENOCD_DIR       := $(TOOLS_DIR)/openocd
OPENOCD_WIN_DIR   := $(TOOLS_DIR)/openocd_win
OPENOCD_BUILD_DIR := $(DL_DIR)/openocd-build

.PHONY: openocd_install
openocd_install: | $(DL_DIR) $(TOOLS_DIR)
openocd_install: OPENOCD_URL  := http://sourceforge.net/projects/openocd/files/openocd/0.5.0/openocd-0.5.0.tar.bz2/download
openocd_install: OPENOCD_FILE := openocd-0.5.0.tar.bz2
openocd_install: openocd_clean
        # download the source only if it's newer than what we already have
	$(V1) wget -N -P "$(DL_DIR)" --trust-server-name "$(OPENOCD_URL)"

        # extract the source
	$(V1) [ ! -d "$(OPENOCD_BUILD_DIR)" ] || $(RM) -r "$(OPENOCD_BUILD_DIR)"
	$(V1) mkdir -p "$(OPENOCD_BUILD_DIR)"
	$(V1) tar -C $(OPENOCD_BUILD_DIR) -xjf "$(DL_DIR)/$(OPENOCD_FILE)"

        # build and install
	$(V1) mkdir -p "$(OPENOCD_DIR)"
	$(V1) ( \
	  cd $(OPENOCD_BUILD_DIR)/openocd-0.5.0 ; \
	  ./configure --prefix="$(OPENOCD_DIR)" --enable-ft2232_libftdi ; \
	  $(MAKE) --silent ; \
	  $(MAKE) --silent install ; \
	)

        # delete the extracted source when we're done
	$(V1) [ ! -d "$(OPENOCD_BUILD_DIR)" ] || $(RM) -rf "$(OPENOCD_BUILD_DIR)"

.PHONY: ftd2xx_install

FTD2XX_DIR := $(DL_DIR)/ftd2xx

ftd2xx_install: | $(DL_DIR)
ftd2xx_install: FTD2XX_URL  := http://www.ftdichip.com/Drivers/CDM/Beta/CDM20817.zip
ftd2xx_install: FTD2XX_FILE := CDM20817.zip
ftd2xx_install: ftd2xx_clean
        # download the file only if it's newer than what we already have
	$(V0) @echo " DOWNLOAD     $(FTD2XX_URL)"
	$(V1) wget -q -N -P "$(DL_DIR)" "$(FTD2XX_URL)"

        # extract the source
	$(V0) @echo " EXTRACT      $(FTD2XX_FILE) -> $(FTD2XX_DIR)"
	$(V1) mkdir -p "$(FTD2XX_DIR)"
	$(V1) unzip -q -d "$(FTD2XX_DIR)" "$(DL_DIR)/$(FTD2XX_FILE)"

.PHONY: ftd2xx_clean
ftd2xx_clean:
	$(V0) @echo " CLEAN        $(FTD2XX_DIR)"
	$(V1) [ ! -d "$(FTD2XX_DIR)" ] || $(RM) -r "$(FTD2XX_DIR)"

.PHONY: ftd2xx_install

LIBUSB_WIN_DIR := $(DL_DIR)/libusb-win32-bin-1.2.6.0

libusb_win_install: | $(DL_DIR)
libusb_win_install: LIBUSB_WIN_URL  := http://sourceforge.net/projects/libusb-win32/files/libusb-win32-releases/1.2.6.0/libusb-win32-bin-1.2.6.0.zip/download
libusb_win_install: LIBUSB_WIN_FILE := libusb-win32-bin-1.2.6.0.zip
libusb_win_install: libusb_win_clean
        # download the file only if it's newer than what we already have
	$(V0) @echo " DOWNLOAD     $(LIBUSB_WIN_URL)"
	$(V1) wget -q -N -P "$(DL_DIR)" --trust-server-name "$(LIBUSB_WIN_URL)"

        # extract the source
	$(V0) @echo " EXTRACT      $(LIBUSB_WIN_FILE) -> $(LIBUSB_WIN_DIR)"
	$(V1) mkdir -p "$(LIBUSB_WIN_DIR)"
	$(V1) unzip -q -d "$(DL_DIR)" "$(DL_DIR)/$(LIBUSB_WIN_FILE)"

        # fixup .h file needed by openocd build
	$(V0) @echo " FIXUP        $(LIBUSB_WIN_DIR)"
	$(V1) ln -s "$(LIBUSB_WIN_DIR)/include/lusb0_usb.h" "$(LIBUSB_WIN_DIR)/include/usb.h"

.PHONY: libusb_win_clean
libusb_win_clean:
	$(V0) @echo " CLEAN        $(LIBUSB_WIN_DIR)"
	$(V1) [ ! -d "$(LIBUSB_WIN_DIR)" ] || $(RM) -r "$(LIBUSB_WIN_DIR)"

.PHONY: openocd_git_win_install

openocd_git_win_install: | $(DL_DIR) $(TOOLS_DIR)
openocd_git_win_install: OPENOCD_URL  := git://openocd.git.sourceforge.net/gitroot/openocd/openocd
openocd_git_win_install: OPENOCD_REV  := f1c0133321c8fcadadd10bba5537c0a634eb183b
openocd_git_win_install: openocd_win_clean libusb_win_install ftd2xx_install
        # download the source
	$(V0) @echo " DOWNLOAD     $(OPENOCD_URL) @ $(OPENOCD_REV)"
	$(V1) [ ! -d "$(OPENOCD_BUILD_DIR)" ] || $(RM) -rf "$(OPENOCD_BUILD_DIR)"
	$(V1) mkdir -p "$(OPENOCD_BUILD_DIR)"
	$(V1) git clone --no-checkout $(OPENOCD_URL) "$(DL_DIR)/openocd-build"
	$(V1) ( \
	  cd $(OPENOCD_BUILD_DIR) ; \
	  git checkout -q $(OPENOCD_REV) ; \
	)

        # apply patches
	$(V0) @echo " PATCH        $(OPENOCD_BUILD_DIR)"
	$(V1) ( \
	  cd $(OPENOCD_BUILD_DIR) ; \
	  git apply < $(ROOT_DIR)/flight/Project/OpenOCD/0001-armv7m-remove-dummy-FP-regs-for-new-gdb.patch ; \
	  git apply < $(ROOT_DIR)/flight/Project/OpenOCD/0002-rtos-add-stm32_stlink-to-FreeRTOS-targets.patch ; \
	)

        # build and install
	$(V0) @echo " BUILD        $(OPENOCD_WIN_DIR)"
	$(V1) mkdir -p "$(OPENOCD_WIN_DIR)"
	$(V1) ( \
	  cd $(OPENOCD_BUILD_DIR) ; \
	  ./bootstrap ; \
	  ./configure --enable-maintainer-mode --prefix="$(OPENOCD_WIN_DIR)" \
		--build=i686-pc-linux-gnu --host=i586-mingw32msvc \
		CPPFLAGS=-I$(LIBUSB_WIN_DIR)/include \
		LDFLAGS=-L$(LIBUSB_WIN_DIR)/lib/gcc \
		--enable-ft2232_ftd2xx --with-ftd2xx-win32-zipdir=$(FTD2XX_DIR) \
		--disable-werror \
		--enable-stlink ; \
	  $(MAKE) ; \
	  $(MAKE) install ; \
	)

        # delete the extracted source when we're done
	$(V1) [ ! -d "$(OPENOCD_BUILD_DIR)" ] || $(RM) -rf "$(OPENOCD_BUILD_DIR)"

.PHONY: openocd_win_clean
openocd_win_clean:
	$(V0) @echo " CLEAN        $(OPENOCD_WIN_DIR)"
	$(V1) [ ! -d "$(OPENOCD_WIN_DIR)" ] || $(RM) -r "$(OPENOCD_WIN_DIR)"

.PHONY: openocd_git_install

openocd_git_install: | $(DL_DIR) $(TOOLS_DIR)
openocd_git_install: OPENOCD_URL  := git://openocd.git.sourceforge.net/gitroot/openocd/openocd
openocd_git_install: OPENOCD_REV  := f1c0133321c8fcadadd10bba5537c0a634eb183b
openocd_git_install: openocd_clean
        # download the source
	$(V0) @echo " DOWNLOAD     $(OPENOCD_URL) @ $(OPENOCD_REV)"
	$(V1) [ ! -d "$(OPENOCD_BUILD_DIR)" ] || $(RM) -rf "$(OPENOCD_BUILD_DIR)"
	$(V1) mkdir -p "$(OPENOCD_BUILD_DIR)"
	$(V1) git clone --no-checkout $(OPENOCD_URL) "$(OPENOCD_BUILD_DIR)"
	$(V1) ( \
	  cd $(OPENOCD_BUILD_DIR) ; \
	  git checkout -q $(OPENOCD_REV) ; \
	)

        # apply patches
	$(V0) @echo " PATCH        $(OPENOCD_DIR)"
	$(V1) ( \
	  cd $(OPENOCD_BUILD_DIR) ; \
	  git apply < $(ROOT_DIR)/flight/Project/OpenOCD/0001-armv7m-remove-dummy-FP-regs-for-new-gdb.patch ; \
	  git apply < $(ROOT_DIR)/flight/Project/OpenOCD/0002-rtos-add-stm32_stlink-to-FreeRTOS-targets.patch ; \
	)

        # build and install
	$(V0) @echo " BUILD        $(OPENOCD_DIR)"
	$(V1) mkdir -p "$(OPENOCD_DIR)"
	$(V1) ( \
	  cd $(OPENOCD_BUILD_DIR) ; \
	  ./bootstrap ; \
	  ./configure --enable-maintainer-mode --prefix="$(OPENOCD_DIR)" --enable-ft2232_libftdi --enable-buspirate --enable-stlink ; \
	  $(MAKE) ; \
	  $(MAKE) install ; \
	)

        # delete the extracted source when we're done
	$(V1) [ ! -d "$(OPENOCD_BUILD_DIR)" ] || $(RM) -rf "$(OPENOCD_BUILD_DIR)"

.PHONY: openocd_clean
openocd_clean:
	$(V0) @echo " CLEAN        $(OPENOCD_DIR)"
	$(V1) [ ! -d "$(OPENOCD_DIR)" ] || $(RM) -r "$(OPENOCD_DIR)"

STM32FLASH_DIR := $(TOOLS_DIR)/stm32flash

.PHONY: stm32flash_install
stm32flash_install: STM32FLASH_URL := http://stm32flash.googlecode.com/svn/trunk
stm32flash_install: STM32FLASH_REV := 61
stm32flash_install: stm32flash_clean
        # download the source
	$(V0) @echo " DOWNLOAD     $(STM32FLASH_URL) @ r$(STM32FLASH_REV)"
	$(V1) svn export -q -r "$(STM32FLASH_REV)" "$(STM32FLASH_URL)" "$(STM32FLASH_DIR)"

        # build
	$(V0) @echo " BUILD        $(STM32FLASH_DIR)"
	$(V1) $(MAKE) --silent -C $(STM32FLASH_DIR) all

.PHONY: stm32flash_clean
stm32flash_clean:
	$(V0) @echo " CLEAN        $(STM32FLASH_DIR)"
	$(V1) [ ! -d "$(STM32FLASH_DIR)" ] || $(RM) -r "$(STM32FLASH_DIR)"

DFUUTIL_DIR := $(TOOLS_DIR)/dfu-util

.PHONY: dfuutil_install
dfuutil_install: DFUUTIL_URL  := http://dfu-util.gnumonks.org/releases/dfu-util-0.5.tar.gz
dfuutil_install: DFUUTIL_FILE := $(notdir $(DFUUTIL_URL))
dfuutil_install: | $(DL_DIR) $(TOOLS_DIR)
dfuutil_install: dfuutil_clean
        # download the source
	$(V0) @echo " DOWNLOAD     $(DFUUTIL_URL)"
	$(V1) wget -N -P "$(DL_DIR)" "$(DFUUTIL_URL)"

        # extract the source
	$(V0) @echo " EXTRACT      $(DFUUTIL_FILE)"
	$(V1) [ ! -d "$(DL_DIR)/dfuutil-build" ] || $(RM) -r "$(DL_DIR)/dfuutil-build"
	$(V1) mkdir -p "$(DL_DIR)/dfuutil-build"
	$(V1) tar -C $(DL_DIR)/dfuutil-build -xf "$(DL_DIR)/$(DFUUTIL_FILE)"

        # build
	$(V0) @echo " BUILD        $(DFUUTIL_DIR)"
	$(V1) mkdir -p "$(DFUUTIL_DIR)"
	$(V1) ( \
	  cd $(DL_DIR)/dfuutil-build/dfu-util-0.5 ; \
	  ./configure --prefix="$(DFUUTIL_DIR)" ; \
	  $(MAKE) ; \
	  $(MAKE) install ; \
	)

.PHONY: dfuutil_clean
dfuutil_clean:
	$(V0) @echo " CLEAN        $(DFUUTIL_DIR)"
	$(V1) [ ! -d "$(DFUUTIL_DIR)" ] || $(RM) -r "$(DFUUTIL_DIR)"

# see http://developer.android.com/sdk/ for latest versions
ANDROID_SDK_DIR := $(TOOLS_DIR)/android-sdk-linux
.PHONY: android_sdk_install
android_sdk_install: ANDROID_SDK_URL  := http://dl.google.com/android/android-sdk_r20.0.3-linux.tgz
android_sdk_install: ANDROID_SDK_FILE := $(notdir $(ANDROID_SDK_URL))
# order-only prereq on directory existance:
android_sdk_install: | $(DL_DIR) $(TOOLS_DIR)
android_sdk_install: android_sdk_clean
        # download the source only if it's newer than what we already have
	$(V0) @echo " DOWNLOAD     $(ANDROID_SDK_URL)"
	$(V1) wget --no-check-certificate -N -P "$(DL_DIR)" "$(ANDROID_SDK_URL)"

        # binary only release so just extract it
	$(V0) @echo " EXTRACT      $(ANDROID_SDK_FILE)"
	$(V1) tar -C $(TOOLS_DIR) -xf "$(DL_DIR)/$(ANDROID_SDK_FILE)"

.PHONY: android_sdk_clean
android_sdk_clean:
	$(V0) @echo " CLEAN        $(ANDROID_SDK_DIR)"
	$(V1) [ ! -d "$(ANDROID_SDK_DIR)" ] || $(RM) -r $(ANDROID_SDK_DIR)

.PHONY: android_sdk_update
android_sdk_update:
	$(V0) @echo " UPDATE       $(ANDROID_SDK_DIR)"
	$(ANDROID_SDK_DIR)/tools/android update sdk --no-ui -t platform-tools,android-16,addon-google_apis-google-16

##############################
#
# Set up paths to tools
#
##############################

ifeq ($(shell [ -d "$(QT_SDK_DIR)" ] && echo "exists"), exists)
  QMAKE=$(QT_SDK_DIR)/Desktop/Qt/4.8.1/gcc/bin/qmake
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

ifeq ($(shell [ -d "$(ANDROID_SDK_DIR)" ] && echo "exists"), exists)
  ANDROID := $(ANDROID_SDK_DIR)/tools/android
  ANDROID_DX := $(ANDROID_SDK_DIR)/platform-tools/dx
else
  # not installed, hope it's in the path...
  ANDROID ?= android
  ANDROID_DX ?= dx
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

ifeq ($(V), 1)
GCS_SILENT := 
else
GCS_SILENT := silent
endif

.PHONY: openpilotgcs
openpilotgcs:  uavobjects_gcs
	$(V1) mkdir -p $(BUILD_DIR)/ground/$@
	$(V1) ( cd $(BUILD_DIR)/ground/$@ && \
	  $(QMAKE) $(ROOT_DIR)/ground/openpilotgcs/openpilotgcs.pro -spec $(QT_SPEC) -r CONFIG+="$(GCS_BUILD_CONF) $(GCS_SILENT)" $(GCS_QMAKE_OPTS) && \
	  $(MAKE) -w ; \
	)

.PHONY: openpilotgcs_clean
openpilotgcs_clean:
	$(V0) @echo " CLEAN      $@"
	$(V1) [ ! -d "$(BUILD_DIR)/ground/openpilotgcs" ] || $(RM) -r "$(BUILD_DIR)/ground/openpilotgcs"

ifeq ($(V), 1)
UAVOGEN_SILENT := 
else
UAVOGEN_SILENT := silent
endif

.PHONY: uavobjgenerator
uavobjgenerator:
	$(V1) mkdir -p $(BUILD_DIR)/ground/$@
	$(V1) ( cd $(BUILD_DIR)/ground/$@ && \
	  $(QMAKE) $(ROOT_DIR)/ground/uavobjgenerator/uavobjgenerator.pro -spec $(QT_SPEC) -r CONFIG+="debug $(UAVOGEN_SILENT)" && \
	  $(MAKE) --no-print-directory -w ; \
	)

UAVOBJ_TARGETS := gcs flight python matlab java wireshark
.PHONY:uavobjects
uavobjects:  $(addprefix uavobjects_, $(UAVOBJ_TARGETS))

UAVOBJ_XML_DIR := $(ROOT_DIR)/shared/uavobjectdefinition
UAVOBJ_OUT_DIR := $(BUILD_DIR)/uavobject-synthetics

$(UAVOBJ_OUT_DIR):
	$(V1) mkdir -p $@

uavobjects_%: $(UAVOBJ_OUT_DIR) uavobjgenerator
	$(V1) ( cd $(UAVOBJ_OUT_DIR) && \
	  $(UAVOBJGENERATOR) -$* $(UAVOBJ_XML_DIR) $(ROOT_DIR) ; \
	)

uavobjects_test: $(UAVOBJ_OUT_DIR) uavobjgenerator
	$(V1) $(UAVOBJGENERATOR) -v -none $(UAVOBJ_XML_DIR) $(ROOT_DIR)

uavobjects_clean:
	$(V0) @echo " CLEAN      $@"
	$(V1) [ ! -d "$(UAVOBJ_OUT_DIR)" ] || $(RM) -r "$(UAVOBJ_OUT_DIR)"

################################
#
# Android GCS related components
#
################################


# Build the output directory for the Android GCS build
ANDROIDGCS_OUT_DIR := $(BUILD_DIR)/androidgcs
$(ANDROIDGCS_OUT_DIR):
	$(V1) mkdir -p $@

# Build the asset directory for the android assets
ANDROIDGCS_ASSETS_DIR := $(ANDROIDGCS_OUT_DIR)/assets
$(ANDROIDGCS_ASSETS_DIR)/uavos:
	$(V1) mkdir -p $@

ifeq ($(V), 1)
ANT_QUIET :=
ANDROID_SILENT := 
else
ANT_QUIET := -q
ANDROID_SILENT := -s
endif
.PHONY: androidgcs
androidgcs: uavo-collections_java
	$(V0) @echo " ANDROID   $(call toprel, $(ANDROIDGCS_OUT_DIR))"
	$(V1) mkdir -p $(ANDROIDGCS_OUT_DIR)
	$(V1) $(ANDROID) $(ANDROID_SILENT) update project --target 'Google Inc.:Google APIs:16' --name androidgcs --path ./androidgcs
	$(V1) ant -f ./androidgcs/build.xml \
		$(ANT_QUIET) \
		-Dout.dir="../$(call toprel, $(ANDROIDGCS_OUT_DIR)/bin)" \
		-Dgen.absolute.dir="$(ANDROIDGCS_OUT_DIR)/gen" \
		debug

.PHONY: androidgcs_clean
androidgcs_clean:
	$(V0) @echo " CLEAN      $@"
	$(V1) [ ! -d "$(ANDROIDGCS_OUT_DIR)" ] || $(RM) -r "$(ANDROIDGCS_OUT_DIR)"

# We want to take snapshots of the UAVOs at each point that they change
# to allow the GCS to be compatible with as many versions as possible.
#
# Find the git hashes of each commit that changes uavobjects with:
#   git log --format=%h -- shared/uavobjectdefinition/ | head -n 2
UAVO_GIT_VERSIONS := 684620d 43f85d9

# All versions includes a pseudo collection called "working" which represents
# the UAVOs in the source tree
UAVO_ALL_VERSIONS := $(UAVO_GIT_VERSIONS) srctree

# This is where the UAVO collections are stored
UAVO_COLLECTION_DIR := $(BUILD_DIR)/uavo-collections

# $(1) git hash of a UAVO snapshot
define UAVO_COLLECTION_GIT_TEMPLATE

# Make the output directory that will contain all of the synthetics for the
# uavo collection referenced by the git hash $(1)
$$(UAVO_COLLECTION_DIR)/$(1):
	$$(V1) mkdir -p $$(UAVO_COLLECTION_DIR)/$(1)

# Extract the snapshot of shared/uavobjectdefinition from git hash $(1)
$$(UAVO_COLLECTION_DIR)/$(1)/uavo-xml.tar: | $$(UAVO_COLLECTION_DIR)/$(1)
$$(UAVO_COLLECTION_DIR)/$(1)/uavo-xml.tar:
	$$(V0) @echo " UAVOTAR   $(1)"
	$$(V1) git archive $(1) -o $$@ -- shared/uavobjectdefinition/

# Extract the uavo xml files from our snapshot
$$(UAVO_COLLECTION_DIR)/$(1)/uavo-xml: $$(UAVO_COLLECTION_DIR)/$(1)/uavo-xml.tar
	$$(V0) @echo " UAVOUNTAR $(1)"
	$$(V1) rm -rf $$@
	$$(V1) mkdir -p $$@
	$$(V1) tar -C $$(call toprel, $$@) -xf $$(call toprel, $$<) || rm -rf $$@
endef

# Map the current working directory into the set of UAVO collections
$(UAVO_COLLECTION_DIR)/srctree:
	$(V1) mkdir -p $@

$(UAVO_COLLECTION_DIR)/srctree/uavo-xml: | $(UAVO_COLLECTION_DIR)/srctree
$(UAVO_COLLECTION_DIR)/srctree/uavo-xml: $(UAVOBJ_XML_DIR)
	$(V1) ln -sf $(ROOT_DIR) $(UAVO_COLLECTION_DIR)/srctree/uavo-xml

# $(1) git hash (or symbolic name) of a UAVO snapshot
define UAVO_COLLECTION_BUILD_TEMPLATE

# This leaves us with a (broken) symlink that points to the full sha1sum of the collection
$$(UAVO_COLLECTION_DIR)/$(1)/uavohash: $$(UAVO_COLLECTION_DIR)/$(1)/uavo-xml
        # Compute the sha1 hash for this UAVO collection
        # The sed bit truncates the UAVO hash to 16 hex digits
	$$(V1) python $$(ROOT_DIR)/make/scripts/version-info.py \
			--path=$$(ROOT_DIR) \
			--uavodir=$$(UAVO_COLLECTION_DIR)/$(1)/uavo-xml/shared/uavobjectdefinition \
			--format='$$$${UAVOSHA1TXT}' | \
		sed -e 's|\(................\).*|\1|' > $$@

	$$(V0) @echo " UAVOHASH  $(1) ->" $$$$(cat $$(UAVO_COLLECTION_DIR)/$(1)/uavohash)

# Generate the java uavobjects for this UAVO collection
$$(UAVO_COLLECTION_DIR)/$(1)/java-build/java: $$(UAVO_COLLECTION_DIR)/$(1)/uavohash
	$$(V0) @echo " UAVOJAVA  $(1)   " $$$$(cat $$(UAVO_COLLECTION_DIR)/$(1)/uavohash)
	$$(V1) mkdir -p $$@
	$$(V1) ( \
		cd $$(UAVO_COLLECTION_DIR)/$(1)/java-build && \
		$$(UAVOBJGENERATOR) -java $$(UAVO_COLLECTION_DIR)/$(1)/uavo-xml/shared/uavobjectdefinition $$(ROOT_DIR) ; \
	)

# Build a jar file for this UAVO collection
$$(UAVO_COLLECTION_DIR)/$(1)/java-build/uavobjects.jar: | $$(ANDROIDGCS_ASSETS_DIR)/uavos
$$(UAVO_COLLECTION_DIR)/$(1)/java-build/uavobjects.jar: $$(UAVO_COLLECTION_DIR)/$(1)/java-build/java
	$$(V0) @echo " UAVOJAR   $(1)   " $$$$(cat $$(UAVO_COLLECTION_DIR)/$(1)/uavohash)
	$$(V1) ( \
		HASH=$$$$(cat $$(UAVO_COLLECTION_DIR)/$(1)/uavohash) && \
		cd $$(UAVO_COLLECTION_DIR)/$(1)/java-build && \
		javac java/*.java \
		   $$(ROOT_DIR)/androidgcs/src/org/openpilot/uavtalk/UAVDataObject.java \
		   $$(ROOT_DIR)/androidgcs/src/org/openpilot/uavtalk/UAVObject*.java \
		   $$(ROOT_DIR)/androidgcs/src/org/openpilot/uavtalk/UAVMetaObject.java \
		   -d . && \
		find ./org/openpilot/uavtalk/uavobjects -type f -name '*.class' > classlist.txt && \
		jar cf tmp_uavobjects.jar @classlist.txt && \
		$$(ANDROID_DX) \
			--dex \
			--output $$(ANDROIDGCS_ASSETS_DIR)/uavos/$$$${HASH}.jar \
			tmp_uavobjects.jar && \
		ln -sf $$(ANDROIDGCS_ASSETS_DIR)/uavos/$$$${HASH}.jar uavobjects.jar \
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
	$(V0) @echo " CLEAN  $(UAVO_COLLECTION_DIR)"
	$(V1) [ ! -d "$(UAVO_COLLECTION_DIR)" ] || $(RM) -r $(UAVO_COLLECTION_DIR)

##############################
#
# Flight related components
#
##############################

# $(1) = Canonical board name all in lower case (e.g. coptercontrol)
# $(2) = Name of board used in source tree (e.g. CopterControl)
# $(3) = Short name for board (e.g CC)
define FW_TEMPLATE
.PHONY: $(1) fw_$(1)
$(1): fw_$(1)_opfw
fw_$(1): fw_$(1)_opfw

fw_$(1)_%: uavobjects_flight
	$(V1) mkdir -p $(BUILD_DIR)/fw_$(1)/dep
	$(V1) cd $(ROOT_DIR)/flight/$(2) && \
		$$(MAKE) -r --no-print-directory \
		BOARD_NAME=$(1) \
		BOARD_SHORT_NAME=$(3) \
		BUILD_TYPE=fw \
		TCHAIN_PREFIX="$(ARM_SDK_PREFIX)" \
		REMOVE_CMD="$(RM)" OOCD_EXE="$(OPENOCD)" \
		$$*

.PHONY: $(1)_clean
$(1)_clean: fw_$(1)_clean
fw_$(1)_clean:
	$(V0) @echo " CLEAN      $$@"
	$(V1) $(RM) -fr $(BUILD_DIR)/fw_$(1)
endef

# $(1) = Canonical board name all in lower case (e.g. coptercontrol)
# $(2) = Name of board used in source tree (e.g. CopterControl)
define BL_TEMPLATE
.PHONY: bl_$(1)
bl_$(1): bl_$(1)_bin
bl_$(1)_bino: bl_$(1)_bin

bl_$(1)_%:
	$(V1) mkdir -p $(BUILD_DIR)/bl_$(1)/dep
	$(V1) cd $(ROOT_DIR)/flight/Bootloaders/$(2) && \
		$$(MAKE) -r --no-print-directory \
		BOARD_NAME=$(1) \
		BOARD_SHORT_NAME=$(3) \
		BUILD_TYPE=bl \
		TCHAIN_PREFIX="$(ARM_SDK_PREFIX)" \
		REMOVE_CMD="$(RM)" OOCD_EXE="$(OPENOCD)" \
		$$*

.PHONY: unbrick_$(1)
unbrick_$(1): bl_$(1)_hex
$(if $(filter-out undefined,$(origin UNBRICK_TTY)),
	$(V0) @echo " UNBRICK    $(1) via $$(UNBRICK_TTY)"
	$(V1) $(STM32FLASH_DIR)/stm32flash \
		-w $(BUILD_DIR)/bl_$(1)/bl_$(1).hex \
		-g 0x0 \
		$$(UNBRICK_TTY)
,
	$(V0) @echo
	$(V0) @echo "ERROR: You must specify UNBRICK_TTY=<serial-device> to use for unbricking."
	$(V0) @echo "       eg. $$(MAKE) $$@ UNBRICK_TTY=/dev/ttyUSB0"
)

.PHONY: bl_$(1)_clean
bl_$(1)_clean:
	$(V0) @echo " CLEAN      $$@"
	$(V1) $(RM) -fr $(BUILD_DIR)/bl_$(1)
endef

# $(1) = Canonical board name all in lower case (e.g. coptercontrol)
define BU_TEMPLATE
.PHONY: bu_$(1)
bu_$(1): bu_$(1)_opfw

bu_$(1)_%: bl_$(1)_bino
	$(V1) mkdir -p $(BUILD_DIR)/bu_$(1)/dep
	$(V1) cd $(ROOT_DIR)/flight/Bootloaders/BootloaderUpdater && \
		$$(MAKE) -r --no-print-directory \
		BOARD_NAME=$(1) \
		BOARD_SHORT_NAME=$(3) \
		BUILD_TYPE=bu \
		TCHAIN_PREFIX="$(ARM_SDK_PREFIX)" \
		REMOVE_CMD="$(RM)" OOCD_EXE="$(OPENOCD)" \
		$$*

.PHONY: bu_$(1)_clean
bu_$(1)_clean:
	$(V0) @echo " CLEAN      $$@"
	$(V1) $(RM) -fr $(BUILD_DIR)/bu_$(1)
endef

# $(1) = Canonical board name all in lower case (e.g. coptercontrol)
define EF_TEMPLATE
.PHONY: ef_$(1)
ef_$(1): ef_$(1)_bin

ef_$(1)_%: bl_$(1)_bin fw_$(1)_opfw
	$(V1) mkdir -p $(BUILD_DIR)/ef_$(1)/dep
	$(V1) cd $(ROOT_DIR)/flight/EntireFlash && \
		$$(MAKE) -r --no-print-directory \
		BOARD_NAME=$(1) \
		BOARD_SHORT_NAME=$(3) \
		BUILD_TYPE=ef \
		TCHAIN_PREFIX="$(ARM_SDK_PREFIX)" \
		DFU_CMD="$(DFUUTIL_DIR)/bin/dfu-util" \
		$$*

.PHONY: ef_$(1)_clean
ef_$(1)_clean:
	$(V0) @echo " CLEAN      $$@"
	$(V1) $(RM) -fr $(BUILD_DIR)/ef_$(1)
endef

# When building any of the "all_*" targets, tell all sub makefiles to display
# additional details on each line of output to describe which build and target
# that each line applies to.
ifneq ($(strip $(filter all_%,$(MAKECMDGOALS))),)
export ENABLE_MSG_EXTRA := yes
endif

# When building more than one goal in a single make invocation, also
# enable the extra context for each output line
ifneq ($(word 2,$(MAKECMDGOALS)),)
export ENABLE_MSG_EXTRA := yes
endif

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

ALL_BOARDS := coptercontrol pipxtreme revolution revomini simposix osd
ALL_BOARDS_BU := coptercontrol pipxtreme simposix

# SimPosix only builds on Linux so drop it from the list for
# all other platforms.
ifneq ($(UNAME), Linux)
ALL_BOARDS  := $(filter-out simposix, $(ALL_BOARDS))
ALL_BOARDS_BU  := $(filter-out simposix, $(ALL_BOARDS_BU))
endif

# Friendly names of each board (used to find source tree)
coptercontrol_friendly := CopterControl
pipxtreme_friendly     := PipXtreme
revolution_friendly    := Revolution
revomini_friendly      := RevoMini
simposix_friendly      := SimPosix
osd_friendly           := OSD

# Short names of each board (used to display board name in parallel builds)
coptercontrol_short    := 'cc  '
pipxtreme_short        := 'pipx'
revolution_short       := 'revo'
revomini_short         := 'rm  '
simposix_short         := 'posx'
osd_short              := 'osd '

# Start out assuming that we'll build fw, bl and bu for all boards
FW_BOARDS  := $(ALL_BOARDS)
BL_BOARDS  := $(ALL_BOARDS)
BU_BOARDS  := $(ALL_BOARDS_BU)
EF_BOARDS  := $(ALL_BOARDS)

# FIXME: The BU image doesn't work for F4 boards so we need to
#        filter them out to prevent errors.
BU_BOARDS  := $(filter-out revolution osd, $(BU_BOARDS))

# SimPosix doesn't have a BL, BU or EF target so we need to
# filter them out to prevent errors on the all_flight target.
BL_BOARDS  := $(filter-out simposix, $(BL_BOARDS))
BU_BOARDS  := $(filter-out simposix, $(BU_BOARDS))
EF_BOARDS  := $(filter-out simposix, $(EF_BOARDS))

# SimPosix doesn't have a BL, BU or EF target so we need to
# filter them out to prevent errors on the all_flight target.
BL_BOARDS  := $(filter-out simposix, $(BL_BOARDS))
BU_BOARDS  := $(filter-out simposix, $(BU_BOARDS))
EF_BOARDS  := $(filter-out simposix, $(EF_BOARDS))

# Generate the targets for whatever boards are left in each list
FW_TARGETS := $(addprefix fw_, $(FW_BOARDS))
BL_TARGETS := $(addprefix bl_, $(BL_BOARDS))
BU_TARGETS := $(addprefix bu_, $(BU_BOARDS))
EF_TARGETS := $(addprefix ef_, $(EF_BOARDS))

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

# Expand the bootloader updater rules
$(foreach board, $(ALL_BOARDS), $(eval $(call BU_TEMPLATE,$(board),$($(board)_friendly),$($(board)_short))))

# Expand the firmware rules
$(foreach board, $(ALL_BOARDS), $(eval $(call FW_TEMPLATE,$(board),$($(board)_friendly),$($(board)_short))))

# Expand the bootloader rules
$(foreach board, $(ALL_BOARDS), $(eval $(call BL_TEMPLATE,$(board),$($(board)_friendly),$($(board)_short))))

# Expand the entire-flash rules
$(foreach board, $(ALL_BOARDS), $(eval $(call EF_TEMPLATE,$(board),$($(board)_friendly),$($(board)_short))))

.PHONY: sim_win32
sim_win32: sim_win32_exe

sim_win32_%: uavobjects_flight
	$(V1) mkdir -p $(BUILD_DIR)/sitl_win32
	$(V1) $(MAKE) --no-print-directory \
		-C $(ROOT_DIR)/flight/OpenPilot --file=$(ROOT_DIR)/flight/OpenPilot/Makefile.win32 $*

.PHONY: sim_osx
sim_osx: sim_osx_elf

sim_osx_%: uavobjects_flight
	$(V1) mkdir -p $(BUILD_DIR)/sim_osx
	$(V1) $(MAKE) --no-print-directory \
		-C $(ROOT_DIR)/flight/Revolution --file=$(ROOT_DIR)/flight/Revolution/Makefile.osx $*
##############################
#
# Packaging components
#
##############################

.PHONY: package
package:
	$(V1) cd $@ && $(MAKE) --no-print-directory $@

.PHONY: package_resources
package_resources:
	$(V1) cd package && $(MAKE) --no-print-directory opfw_resource

.PHONY: build-info
build-info:
	$(V1) mkdir -p $(BUILD_DIR)
	$(V1) python $(ROOT_DIR)/make/scripts/version-info.py \
		--path=$(ROOT_DIR) \
		--uavodir=$(ROOT_DIR)/shared/uavobjectdefinition \
		--template="make/templates/$@.txt" \
		--outfile="$(BUILD_DIR)/$@.txt"
