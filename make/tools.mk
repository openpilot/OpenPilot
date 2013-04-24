#
# Installers for tools required by the OpenPilot build system.
# Copyright (c) 2010-2013, The OpenPilot Team, http://www.openpilot.org
#
# NOTE: install targets are not tied to the default goals and must
# be invoked manually. But tool paths set by this file are used
# across the build system.
#
# Ready to use:
#    arm_sdk_install
#    qt_sdk_install
#    mingw_install (Windows only)
#    python_install (Windows only)
#    uncrustify_install
#
# TODO:
#    openocd_install
#    ftd2xx_install
#    libusb_win_install
#    openocd_git_win_install
#    openocd_git_install
#    stm32flash_install
#    dfuutil_install
#    android_sdk_install
#    gtest_install
#
# TODO:
#    help in the top Makefile
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
    $(error $(notdir $(lastword $(MAKEFILE_LIST))) should be included by the top level Makefile)
endif

##############################
#
# Toolchain URLs and directories
#
##############################

ifeq ($(UNAME), Linux)
    ifeq ($(ARCH), x86_64)
        ARM_SDK_URL := http://wiki.openpilot.org/download/attachments/18612236/gcc-arm-none-eabi-4_7-2013q1-20130313-linux-amd64.tar.bz2
        QT_SDK_URL  := "Please install native Qt 4.8.x SDK using package manager"
    else
        ARM_SDK_URL := http://wiki.openpilot.org/download/attachments/18612236/gcc-arm-none-eabi-4_7-2013q1-20130313-linux-i686.tar.bz2
        QT_SDK_URL  := "Please install native Qt 4.8.x SDK using package manager"
    endif
    UNCRUSTIFY_URL := http://wiki.openpilot.org/download/attachments/18612236/uncrustify-0.60.tar.gz
else ifeq ($(UNAME), Darwin)
    ARM_SDK_URL    := http://wiki.openpilot.org/download/attachments/18612236/gcc-arm-none-eabi-4_7-2013q1-20130313-mac.tar.bz2
    QT_SDK_URL     := "Please install native Qt 4.8.x SDK using package manager"
    UNCRUSTIFY_URL := http://wiki.openpilot.org/download/attachments/18612236/uncrustify-0.60.tar.gz
else ifeq ($(UNAME), Windows)
    ARM_SDK_URL    := http://wiki.openpilot.org/download/attachments/18612236/gcc-arm-none-eabi-4_7-2013q1-20130313-windows.tar.bz2
    QT_SDK_URL     := http://wiki.openpilot.org/download/attachments/18612236/qt-4.8.4-windows.tar.bz2
    MINGW_URL      := http://wiki.openpilot.org/download/attachments/18612236/mingw-4.4.0.tar.bz2
    PYTHON_URL     := http://wiki.openpilot.org/download/attachments/18612236/python-2.7.4-windows.tar.bz2
    UNCRUSTIFY_URL := http://wiki.openpilot.org/download/attachments/18612236/uncrustify-0.60-windows.tar.bz2
endif

# Changing PYTHON_DIR, also update it in ground\openpilotgcs\src\app\gcsversioninfo.pri
ARM_SDK_DIR     := $(TOOLS_DIR)/gcc-arm-none-eabi-4_7-2013q1
QT_SDK_DIR      := $(TOOLS_DIR)/qt-4.8.4
MINGW_DIR       := $(TOOLS_DIR)/mingw-4.4.0
PYTHON_DIR      := $(TOOLS_DIR)/python-2.7.4
UNCRUSTIFY_DIR  := $(TOOLS_DIR)/uncrustify-0.60

##############################
#
# All toolchains available for the platform
#
##############################

ALL_SDK_TARGETS := arm_sdk qt_sdk uncrustify
ifeq ($(UNAME), Windows)
    ALL_SDK_TARGETS += mingw python
endif

.PHONY: all_sdk_install all_sdk_clean all_sdk_distclean all_sdk_version
all_sdk_install:   $(addsuffix _install,$(ALL_SDK_TARGETS))
all_sdk_clean:     $(addsuffix _clean,$(ALL_SDK_TARGETS))
all_sdk_distclean: $(addsuffix _distclean,$(ALL_SDK_TARGETS))
all_sdk_version:   $(addsuffix _version,$(ALL_SDK_TARGETS))

##############################
#
# Misc host tools
#
##############################

# Used by other makefiles
export MKDIR	:= mkdir
export CP	:= cp
export RM	:= rm
export LN	:= ln
export CAT	:= cat
export CUT	:= cut
export SED	:= sed

# Used only by this Makefile
TAR		:= tar
ANT		:= ant
JAVAC		:= javac
JAR		:= jar
GIT		:= git
CURL		:= curl
OPENSSL		:= openssl

# Echo in recipes is a bit tricky in a Windows Git Bash window in some cases.
# It does not work if make started under msysGit installed into a path with spaces.
ifneq ($(UNAME), Windows)
    export ECHO	:= echo
else
#   export ECHO := $(PYTHON) -c "import sys; print(' '.join(sys.argv[1:]))"
    export ECHO	:= echo
endif

# Test if quotes are needed for the echo command
ifeq ($(shell $(ECHO) "test"), test)
    export QUOTE := '
# This line is just to clear out the single quote above '
else
    export QUOTE :=
endif

# Command to extract version info data from the repository and source tree
export VERSION_INFO = $(PYTHON) $(ROOT_DIR)/make/scripts/version-info.py --path=$(ROOT_DIR)

##############################
#
# Misc settings
#
##############################

# Define messages
MSG_VERIFYING        = $(QUOTE) VERIFY     $(QUOTE)
MSG_DOWNLOADING      = $(QUOTE) DOWNLOAD   $(QUOTE)
MSG_CHECKSUMMING     = $(QUOTE) MD5        $(QUOTE)
MSG_EXTRACTING       = $(QUOTE) EXTRACT    $(QUOTE)
MSG_CONFIGURING      = $(QUOTE) CONFIGURE  $(QUOTE)
MSG_BUILDING         = $(QUOTE) BUILD      $(QUOTE)
MSG_INSTALLING       = $(QUOTE) INSTALL    $(QUOTE)
MSG_CLEANING         = $(QUOTE) CLEAN      $(QUOTE)
MSG_DISTCLEANING     = $(QUOTE) DISTCLEAN  $(QUOTE)
MSG_NOTICE           = $(QUOTE) NOTE       $(QUOTE)

# Verbosity level
ifeq ($(V), 1)
    CURL_OPTIONS :=
else
    CURL_OPTIONS := --silent
endif

# MSYS tar workaround
ifeq ($(UNAME), Windows)
    TAR_OPTIONS := --force-local
else
    TAR_OPTIONS :=
endif

# Print some useful notes for *_install targets
ifneq ($(strip $(filter $(addsuffix _install,all_sdk $(ALL_SDK_TARGETS)),$(MAKECMDGOALS))),)
    ifneq ($(shell $(CURL) --version >/dev/null 2>&1 && $(ECHO) "found"), found)
        $(error Please install curl first ('apt-get install curl' or similar))
    endif
    $(info $(EMPTY) NOTE        Use 'make all_sdk_distclean' to remove installation files)
    $(info $(EMPTY) NOTE        Use 'make all_sdk_version' to check toolchain versions)
    $(info $(EMPTY) NOTE        Add 'V=1' to make command line to diagnose make problems)
endif

##############################
#
# Cross-platform MD5 check template
#  $(1) = file name without quotes
#  $(2) = string compare operator, e.g. = or !=
#
##############################
define MD5_CHECK_TEMPLATE
"`test -f \"$(1)\" && $(OPENSSL) dgst -md5 \"$(1)\" | $(CUT) -f2 -d' '`" $(2) "`$(CUT) -f1 -d' ' < \"$(1).md5\"`"
endef

##############################
#
# Common tool install template
#  $(1) = tool name
#  $(2) = tool extract/build directory
#  $(3) = tool distribution URL
#  $(4) = tool distribution file
#  $(5) = optional extra build recipes template
#  $(6) = optional extra clean recipes template
#
##############################

define TOOL_INSTALL_TEMPLATE

.PHONY: $(addprefix $(1)_, install clean distclean)

$(1)_install: $(1)_clean | $(DL_DIR) $(TOOLS_DIR)
	@$(ECHO) $(MSG_VERIFYING) $$(call toprel, $(DL_DIR)/$(4))
	$(V1) ( \
		cd "$(DL_DIR)" && \
		$(CURL) $(CURL_OPTIONS) -o "$(DL_DIR)/$(4).md5" "$(3).md5" && \
		if [ $(call MD5_CHECK_TEMPLATE,$(DL_DIR)/$(4),!=) ]; then \
			$(ECHO) $(MSG_DOWNLOADING) $(3) && \
			$(CURL) $(CURL_OPTIONS) -o "$(DL_DIR)/$(4)" "$(3)" && \
			$(ECHO) $(MSG_CHECKSUMMING) $$(call toprel, $(DL_DIR)/$(4)) && \
			[ $(call MD5_CHECK_TEMPLATE,$(DL_DIR)/$(4),=) ]; \
		fi; \
	)

	@$(ECHO) $(MSG_EXTRACTING) $$(call toprel, $(2))
	$(V1) $(MKDIR) -p $$(call toprel, $(dir $(2)))
	$(V1) $(TAR) $(TAR_OPTIONS) -C $$(call toprel, $(dir $(2))) -xf $$(call toprel, $(DL_DIR)/$(4))

	$(5)

$(1)_clean:
	@$(ECHO) $(MSG_CLEANING) $$(call toprel, $(2))
	$(V1) [ ! -d "$(2)" ] || $(RM) -rf "$(2)"

	$(6)

$(1)_distclean:
	@$(ECHO) $(MSG_DISTCLEANING) $$(call toprel, $(DL_DIR)/$(4))
	$(V1) [ ! -f "$(DL_DIR)/$(4)" ]     || $(RM) "$(DL_DIR)/$(4)"
	$(V1) [ ! -f "$(DL_DIR)/$(4).md5" ] || $(RM) "$(DL_DIR)/$(4).md5"

endef

##############################
#
# ARM SDK
#
##############################

$(eval $(call TOOL_INSTALL_TEMPLATE,arm_sdk,$(ARM_SDK_DIR),$(ARM_SDK_URL),$(notdir $(ARM_SDK_URL))))

ifeq ($(shell [ -d "$(ARM_SDK_DIR)" ] && $(ECHO) "exists"), exists)
    export ARM_SDK_PREFIX := $(ARM_SDK_DIR)/bin/arm-none-eabi-
else
    # not installed, hope it's in the path...
    # $(info $(EMPTY) WARNING     $(call toprel, $(ARM_SDK_DIR)) not found (make arm_sdk_install), using system PATH)
    export ARM_SDK_PREFIX ?= arm-none-eabi-
endif

.PHONY: arm_sdk_version
arm_sdk_version:
	-$(V1) $(ARM_SDK_PREFIX)gcc --version | head -n1

# Template to check ARM toolchain version before building targets
define ARM_GCC_VERSION_CHECK_TEMPLATE
	if ! $(ARM_SDK_PREFIX)gcc --version --specs=nano.specs >/dev/null 2>&1; then \
		$(ECHO) $(MSG_NOTICE) Please install ARM toolchain 4.7+ using \'make arm_sdk_install\' && \
		$(ECHO) $(MSG_NOTICE) Older ARM SDKs do not support new \'--specs=nano.specs\' option && \
		exit 1; \
	fi
endef

##############################
#
# Qt SDK
#
# Windows:  native binary package has been provided
# Linux:    user should install native Qt SDK package
# Mac OS X: user should install native Qt SDK package
#
##############################

ifeq ($(UNAME), Windows)

define QT_SDK_CONFIGURE_TEMPLATE
	@$(ECHO) $(MSG_CONFIGURING) $(call toprel, $(QT_SDK_DIR))
	$(V1) $(ECHO) $(QUOTE)[Paths]$(QUOTE) > $(QT_SDK_DIR)/bin/qt.conf
	$(V1) $(ECHO) $(QUOTE)Prefix = $(QT_SDK_DIR)$(QUOTE) >> $(QT_SDK_DIR)/bin/qt.conf
endef

    $(eval $(call TOOL_INSTALL_TEMPLATE,qt_sdk,$(QT_SDK_DIR),$(QT_SDK_URL),$(notdir $(QT_SDK_URL)),$(QT_SDK_CONFIGURE_TEMPLATE)))

else

.PHONY: qt_sdk_install
qt_sdk_install:
	@$(ECHO) $(MSG_NOTICE) --------------------------------------------------------
	@$(ECHO) $(MSG_NOTICE) Please install native Qt 4.8.x SDK using package manager
	@$(ECHO) $(MSG_NOTICE) --------------------------------------------------------

.PHONY: qt_sdk_clean
qt_sdk_clean:

.PHONY: qt_sdk_distclean
qt_sdk_distclean:

endif

ifeq ($(shell [ -d "$(QT_SDK_DIR)" ] && $(ECHO) "exists"), exists)
    export QMAKE := $(QT_SDK_DIR)/bin/qmake

    # set Qt library search path
    ifeq ($(UNAME), Windows)
        export PATH := $(QT_SDK_DIR)/bin:$(PATH)
    else
        export LD_LIBRARY_PATH := $(QT_SDK_DIR)/lib:$(LD_LIBRARY_PATH)
    endif
else
    # not installed, hope it's in the path...
    # $(info $(EMPTY) WARNING     $(call toprel, $(QT_SDK_DIR)) not found (make qt_sdk_install), using system PATH)
    QMAKE ?= qmake
endif

.PHONY: qt_sdk_version
qt_sdk_version:
	-$(V1) $(QMAKE) --version | tail -1

##############################
#
# MinGW
#
##############################

ifeq ($(UNAME), Windows)

$(eval $(call TOOL_INSTALL_TEMPLATE,mingw,$(MINGW_DIR),$(MINGW_URL),$(notdir $(MINGW_URL))))

ifeq ($(shell [ -d "$(MINGW_DIR)" ] && $(ECHO) "exists"), exists)
    # set MinGW binary and library paths (QTMINGW is used by qmake, do not rename)
    export QTMINGW := $(MINGW_DIR)/bin
    export PATH    := $(QTMINGW):$(PATH)
else
    # not installed, use host gcc compiler
    # $(info $(EMPTY) WARNING     $(call toprel, $(MINGW_DIR)) not found (make mingw_install), using system PATH)
endif

.PHONY: mingw_version
mingw_version:
	-$(V1) gcc --version | head -n1

.PHONY: gcc_version
gcc_version: mingw_version

else # Linux or Mac

all_sdk_version: gcc_version

.PHONY: gcc_version
gcc_version:
	-$(V1) gcc --version | head -n1

endif

##############################
#
# Python
#
##############################

ifeq ($(UNAME), Windows)

$(eval $(call TOOL_INSTALL_TEMPLATE,python,$(PYTHON_DIR),$(PYTHON_URL),$(notdir $(PYTHON_URL))))

else # Linux or Mac

all_sdk_version: python_version

endif

ifeq ($(shell [ -d "$(PYTHON_DIR)" ] && $(ECHO) "exists"), exists)
    export PYTHON := $(PYTHON_DIR)/python
    export PATH   := $(PYTHON_DIR):$(PATH)
else
    # not installed, hope it's in the path...
    # $(info $(EMPTY) WARNING     $(call toprel, $(PYTHON_DIR)) not found (make python_install), using system PATH)
    export PYTHON := python
endif

.PHONY: python_version
python_version:
	-$(V1) $(PYTHON) --version

##############################
#
# Uncrustify
#
##############################

ifeq ($(UNAME), Windows)

$(eval $(call TOOL_INSTALL_TEMPLATE,uncrustify,$(UNCRUSTIFY_DIR),$(UNCRUSTIFY_URL),$(notdir $(UNCRUSTIFY_URL))))

else # Linux or Mac

UNCRUSTIFY_BUILD_DIR := $(BUILD_DIR)/$(notdir $(UNCRUSTIFY_DIR))

define UNCRUSTIFY_BUILD_TEMPLATE
	$(V1) ( \
		$(ECHO) $(MSG_CONFIGURING) $(call toprel, $(UNCRUSTIFY_BUILD_DIR)) && \
		cd $(UNCRUSTIFY_BUILD_DIR) && \
		./configure --prefix="$(UNCRUSTIFY_DIR)" && \
		$(ECHO) $(MSG_BUILDING) $(call toprel, $(UNCRUSTIFY_BUILD_DIR)) && \
		$(MAKE) --silent && \
		$(ECHO) $(MSG_INSTALLING) $(call toprel, $(UNCRUSTIFY_DIR)) && \
		$(MAKE) --silent install-strip \
	)
	@$(ECHO) $(MSG_CLEANING) $(call toprel, $(UNCRUSTIFY_BUILD_DIR))
	-$(V1) [ ! -d "$(UNCRUSTIFY_BUILD_DIR)" ] || $(RM) -rf "$(UNCRUSTIFY_BUILD_DIR)"
endef

define UNCRUSTIFY_CLEAN_TEMPLATE
	-$(V1) [ ! -d "$(UNCRUSTIFY_DIR)" ] || $(RM) -rf "$(UNCRUSTIFY_DIR)"
endef

$(eval $(call TOOL_INSTALL_TEMPLATE,uncrustify,$(UNCRUSTIFY_BUILD_DIR),$(UNCRUSTIFY_URL),$(notdir $(UNCRUSTIFY_URL)),$(UNCRUSTIFY_BUILD_TEMPLATE),$(UNCRUSTIFY_CLEAN_TEMPLATE)))

endif

ifeq ($(shell [ -d "$(UNCRUSTIFY_DIR)" ] && $(ECHO) "exists"), exists)
    export UNCRUSTIFY := $(UNCRUSTIFY_DIR)/bin/uncrustify
else
    # not installed, hope it's in the path...
    # $(info $(EMPTY) WARNING     $(call toprel, $(UNCRUSTIFY_DIR)) not found (make uncrustify_install), using system PATH)
    export UNCRUSTIFY := uncrustify
endif

.PHONY: uncrustify_version
uncrustify_version:
	-$(V1) $(UNCRUSTIFY) --version





##############################
#
# TODO: code below is not revised yet
#
##############################

# Set up openocd tools
OPENOCD_DIR       := $(TOOLS_DIR)/openocd
OPENOCD_WIN_DIR   := $(TOOLS_DIR)/openocd_win
OPENOCD_BUILD_DIR := $(DL_DIR)/openocd-build

.PHONY: openocd_install
openocd_install: | $(DL_DIR) $(TOOLS_DIR)
openocd_install: OPENOCD_URL  := http://sourceforge.net/projects/openocd/files/openocd/0.6.1/openocd-0.6.1.tar.bz2/download
openocd_install: OPENOCD_FILE := openocd-0.6.1.tar.bz2
openocd_install: openocd_clean
        # download the source only if it's newer than what we already have
	$(V1) $(WGET) -N -P "$(DL_DIR)" --trust-server-name "$(OPENOCD_URL)"

        # extract the source
	$(V1) [ ! -d "$(OPENOCD_BUILD_DIR)" ] || $(RM) -r "$(OPENOCD_BUILD_DIR)"
	$(V1) mkdir -p "$(OPENOCD_BUILD_DIR)"
	$(V1) tar -C $(OPENOCD_BUILD_DIR) -xjf "$(DL_DIR)/$(OPENOCD_FILE)"

        # apply patches
	$(V0) @echo " PATCH        $(OPENOCD_DIR)"
	$(V1) ( \
	  cd $(OPENOCD_BUILD_DIR)/openocd-0.6.1 ; \
	  patch -p1 < $(ROOT_DIR)/flight/Project/OpenOCD/0001-armv7m-remove-dummy-FP-regs-for-new-gdb.patch ; \
	  patch -p1 < $(ROOT_DIR)/flight/Project/OpenOCD/0002-rtos-add-stm32_stlink-to-FreeRTOS-targets.patch ; \
	)

        # build and install
	$(V1) mkdir -p "$(OPENOCD_DIR)"
	$(V1) ( \
	  cd $(OPENOCD_BUILD_DIR)/openocd-0.6.1 ; \
	  ./configure --prefix="$(OPENOCD_DIR)" --enable-ft2232_libftdi --enable-stlink ; \
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
	$(V1) $(WGET) -q -N -P "$(DL_DIR)" "$(FTD2XX_URL)"

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
	$(V1) $(WGET) -q -N -P "$(DL_DIR)" --trust-server-name "$(LIBUSB_WIN_URL)"

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
dfuutil_install: DFUUTIL_URL  := http://dfu-util.gnumonks.org/releases/dfu-util-0.7.tar.gz
dfuutil_install: DFUUTIL_FILE := $(notdir $(DFUUTIL_URL))
dfuutil_install: | $(DL_DIR) $(TOOLS_DIR)
dfuutil_install: dfuutil_clean
        # download the source
	$(V0) @echo " DOWNLOAD     $(DFUUTIL_URL)"
	$(V1) $(WGET) -N -P "$(DL_DIR)" "$(DFUUTIL_URL)"

        # extract the source
	$(V0) @echo " EXTRACT      $(DFUUTIL_FILE)"
	$(V1) [ ! -d "$(DL_DIR)/dfuutil-build" ] || $(RM) -r "$(DL_DIR)/dfuutil-build"
	$(V1) mkdir -p "$(DL_DIR)/dfuutil-build"
	$(V1) tar -C $(DL_DIR)/dfuutil-build -xf "$(DL_DIR)/$(DFUUTIL_FILE)"

        # build
	$(V0) @echo " BUILD        $(DFUUTIL_DIR)"
	$(V1) mkdir -p "$(DFUUTIL_DIR)"
	$(V1) ( \
	  cd $(DL_DIR)/dfuutil-build/dfu-util-0.7 ; \
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
	$(V1) $(WGET) --no-check-certificate -N -P "$(DL_DIR)" "$(ANDROID_SDK_URL)"

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

# Set up Google Test (gtest) tools
GTEST_DIR       := $(TOOLS_DIR)/gtest-1.6.0

.PHONY: gtest_install
gtest_install: | $(DL_DIR) $(TOOLS_DIR)
gtest_install: GTEST_URL  := http://googletest.googlecode.com/files/gtest-1.6.0.zip
gtest_install: GTEST_FILE := $(notdir $(GTEST_URL))
gtest_install: gtest_clean
        # download the file unconditionally since google code gives back 404
        # for HTTP HEAD requests which are used when using the wget -N option
	$(V1) [ ! -f "$(DL_DIR)/$(GTEST_FILE)" ] || $(RM) -f "$(DL_DIR)/$(GTEST_FILE)"
	$(V1) $(WGET) -P "$(DL_DIR)" --trust-server-name "$(GTEST_URL)"

        # extract the source
	$(V1) [ ! -d "$(GTEST_DIR)" ] || $(RM) -rf "$(GTEST_DIR)"
	$(V1) mkdir -p "$(GTEST_DIR)"
	$(V1) unzip -q -d "$(TOOLS_DIR)" "$(DL_DIR)/$(GTEST_FILE)"

.PHONY: gtest_clean
gtest_clean:
	$(V0) @echo " CLEAN        $(GTEST_DIR)"
	$(V1) [ ! -d "$(GTEST_DIR)" ] || $(RM) -rf "$(GTEST_DIR)"

##############################
#
# TODO: these defines will go to tool install sections
#
##############################

ifeq ($(shell [ -d "$(OPENOCD_DIR)" ] && $(ECHO) "exists"), exists)
    export OPENOCD := $(OPENOCD_DIR)/bin/openocd
else
    # not installed, hope it's in the path...
    export OPENOCD ?= openocd
endif

ifeq ($(shell [ -d "$(ANDROID_SDK_DIR)" ] && $(ECHO) "exists"), exists)
    ANDROID    := $(ANDROID_SDK_DIR)/tools/android
    ANDROID_DX := $(ANDROID_SDK_DIR)/platform-tools/dx
else
    # not installed, hope it's in the path...
    ANDROID    ?= android
    ANDROID_DX ?= dx
endif
