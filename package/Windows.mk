#
# Windows-specific packaging script
#

ifndef OPENPILOT_IS_COOL
    $(error Top level Makefile must be used to build this target)
endif

VERSION_CMD   := $(VERSION_INFO)

NSIS_OPTS     := /V3
NSIS_WINX86   := $(ROOT_DIR)/package/winx86
NSIS_SCRIPT   := $(NSIS_WINX86)/openpilotgcs.nsi
NSIS_TEMPLATE := $(NSIS_WINX86)/openpilotgcs.tpl
NSIS_HEADER   := $(OPGCSSYNTHDIR)/openpilotgcs.nsh

.PHONY: package
package: openpilotgcs uavobjects_matlab | $(PACKAGE_DIR)
ifneq ($(GCS_BUILD_CONF),release)
	# We can only package release builds
	$(error Packaging is currently supported for release builds only)
endif
	$(V1) mkdir -p "$(dir $(NSIS_HEADER))"
	$(VERSION_CMD) \
		--template='$(NSIS_TEMPLATE)' \
		--outfile='$(NSIS_HEADER)' \
	        OP_BIG_NAME='$(OP_BIG_NAME)' \
	        GCS_BIG_NAME='$(GCS_BIG_NAME)' \
	        GCS_SMALL_NAME='$(GCS_SMALL_NAME)' \
		PACKAGE_LBL='$(PACKAGE_LBL)' \
		PACKAGE_NAME='$(PACKAGE_NAME)' \
		PACKAGE_SEP='$(PACKAGE_SEP)'
	$(V1) echo "Building Windows installer, please wait..."
	$(V1) echo "If you have a script error in line 1 - use Unicode NSIS 2.46+"
	$(V1) echo "  http://www.scratchpaper.com"
	$(NSIS) $(NSIS_OPTS) $(NSIS_SCRIPT)
