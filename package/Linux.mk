#
# Linux-specific packaging script
#

ifndef OPENPILOT_IS_COOL
    $(error Top level Makefile must be used to build this target)
endif

# Update this number for every formal release.  The Deb packaging system relies on this to know to update a
# package or not.  Otherwise, the user has to uninstall first.
# Until we do that, package name does NOT include $(VERNUM) and uses $(PACKAGE_LBL) only
VERNUM			:= 0.1.0
VERSION_FULL		:= $(VERNUM)-$(PACKAGE_LBL)
DEB_DIR			:= $(ROOT_DIR)/package/linux/debian
DEB_BUILD_DIR		:= $(ROOT_DIR)/debian

SED_DATE_STRG		= $(shell date -R)
SED_SCRIPT		= s/<VERSION>/$(VERNUM)/;s/<DATE>/$(SED_DATE_STRG)/

DEB_PLATFORM		:= amd64
MACHINE_TYPE		:= $(shell uname -m)
ifneq ($(MACHINE_TYPE), x86_64)
 DEB_PLATFORM		:= i386
endif
DEB_PACKAGE_NAME	:= openpilot_$(VERNUM)_$(DEB_PLATFORM)

.PHONY: package
package:
	$(V1) echo "Building Linux package, please wait..."
	$(V1) cp -r $(DEB_DIR) $(DEB_BUILD_DIR)
	$(V1)sed -i -e "$(SED_SCRIPT)" $(DEB_BUILD_DIR)/changelog
	$(V1) cd .. && dpkg-buildpackage -b -us -uc
	$(V1) mv $(ROOT_DIR)/../$(DEB_PACKAGE_NAME).deb $(BUILD_DIR)/$(DEB_PACKAGE_NAME).deb
	$(V1) mv $(ROOT_DIR)/../$(DEB_PACKAGE_NAME).changes $(BUILD_DIR)/$(DEB_PACKAGE_NAME).changes
	$(V1) rm -rf $(DEB_BUILD_DIR)
