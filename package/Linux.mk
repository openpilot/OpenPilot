#
# Linux-specific packaging script
#

ifndef OPENPILOT_IS_COOL
    $(error Top level Makefile must be used to build this target)
endif

DEB_VER			:= $(PACKAGE_LBL)-1
DEB_DIR			:= $(ROOT_DIR)/package/linux/debian
DEB_BUILD_DIR		:= $(ROOT_DIR)/debian

SED_DATE_STRG		= $(shell date -R)
SED_SCRIPT		= s/<VERSION>/$(DEB_VER)/;s/<DATE>/$(SED_DATE_STRG)/

DEB_ARCH		:= $(shell dpkg --print-architecture)
DEB_PACKAGE_NAME	:= openpilot_$(DEB_VER)_$(DEB_ARCH)

.PHONY: package
package:
	$(V1) echo "Building Linux package, please wait..."
	$(V1) cp -rL $(DEB_DIR) $(DEB_BUILD_DIR)
	$(V1) sed -i -e "$(SED_SCRIPT)" $(DEB_BUILD_DIR)/changelog
	$(V1) dpkg-buildpackage -b -us -uc
	$(V1) mv $(ROOT_DIR)/../$(DEB_PACKAGE_NAME).deb $(BUILD_DIR)/$(DEB_PACKAGE_NAME).deb
	$(V1) mv $(ROOT_DIR)/../$(DEB_PACKAGE_NAME).changes $(BUILD_DIR)/$(DEB_PACKAGE_NAME).changes
	$(V1) rm -rf $(DEB_BUILD_DIR)
