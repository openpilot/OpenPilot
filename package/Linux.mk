#
# Linux-specific packaging script
#

ifndef OPENPILOT_IS_COOL
    $(error Top level Makefile must be used to build this target)
endif

DEB_DIST             := unstable
# Instead of RELEASE-15.01-RC1 debian wants 15.01~RC1
UPSTREAM_VER         := $(subst -,~,$(subst RELEASE-,,$(PACKAGE_LBL)))
DEB_REV              := 1
DEB_NAME             := openpilot
DEB_ORIG_SRC         := $(PACKAGE_DIR)/$(DEB_NAME)_$(UPSTREAM_VER).orig.tar.gz
DEB_PACKAGE_DIR      := $(PACKAGE_DIR)/$(DEB_NAME)-$(UPSTREAM_VER)
DEB_ARCH             := $(shell dpkg --print-architecture)
DEB_PACKAGE_NAME     := $(DEB_NAME)_$(UPSTREAM_VER)-$(DEB_REV)_$(DEB_ARCH)
DEB_DIR              := package/linux/debian

SED_DATE_STRG         = $(shell date -R)
SED_SCRIPT            = s/<VERSION>/$(UPSTREAM_VER)-$(DEB_REV)/;s/<DATE>/$(SED_DATE_STRG)/;s/<DIST>/$(DEB_DIST)/


.PHONY: package
package: debian
	@$(ECHO) "Building Linux package, please wait..."
	# Override clean and build because OP has already performed them.
	$(V1) printf "override_dh_auto_clean:\noverride_dh_auto_build:\n\t#\n" >> debian/rules
	$(V1) dpkg-buildpackage -b -us -uc
	$(V1) mv $(ROOT_DIR)/../$(DEB_PACKAGE_NAME).deb $(PACKAGE_DIR)
	$(V1) mv $(ROOT_DIR)/../$(DEB_PACKAGE_NAME).changes $(PACKAGE_DIR)
	$(V1) rm -r debian

debian: $(DEB_DIR)
	$(V1) rm -rf debian
	$(V1) cp -rL $(DEB_DIR) debian
	$(V1) sed -i -e "$(SED_SCRIPT)" debian/changelog

.PHONY: package_src
package_src:  $(DEB_ORIG_SRC_NAME) $(DEB_PACKAGE_DIR)
	$(V1) cd $(DEB_PACKAGE_DIR) && dpkg-buildpackage -S -us -uc

$(DEB_ORIG_SRC): $(DIST_NAME).gz | $(PACKAGE_DIR)
	$(V1) cp $(DIST_NAME).gz $(DEB_ORIG_SRC)

$(DEB_PACKAGE_DIR): $(DEB_ORIG_SRC) debian | $(PACKAGE_DIR)
	$(V1) tar -xf $(DEB_ORIG_SRC) -C $(PACKAGE_DIR)
	$(V1) mv debian $(PACKAGE_DIR)/$(PACKAGE_NAME)
	$(V1) rm -rf $(DEB_PACKAGE_DIR) && mv $(PACKAGE_DIR)/$(PACKAGE_NAME) $(DEB_PACKAGE_DIR)

##############################
#
# Install OpenPilot
#
##############################
prefix  := /usr/local
bindir  := $(prefix)/bin
libdir  := $(prefix)/lib
datadir := $(prefix)/share

INSTALL = cp -a --no-preserve=ownership
LN = ln
LN_S = ln -s

.PHONY: install
install:
	@$(ECHO) " INSTALLING GCS TO $(DESTDIR)/)"
	$(V1) $(MKDIR) -p $(DESTDIR)$(bindir)
	$(V1) $(MKDIR) -p $(DESTDIR)$(libdir)
	$(V1) $(MKDIR) -p $(DESTDIR)$(datadir)
	$(V1) $(MKDIR) -p $(DESTDIR)$(datadir)/applications
	$(V1) $(MKDIR) -p $(DESTDIR)$(datadir)/pixmaps
	$(V1) $(MKDIR) -p $(DESTDIR)$(udevdir)
	$(V1) $(INSTALL) $(BUILD_DIR)/openpilotgcs_$(GCS_BUILD_CONF)/bin/openpilotgcs $(DESTDIR)$(bindir)
	$(V1) $(INSTALL) $(BUILD_DIR)/openpilotgcs_$(GCS_BUILD_CONF)/bin/udp_test $(DESTDIR)$(bindir)
	$(V1) $(INSTALL) $(BUILD_DIR)/openpilotgcs_$(GCS_BUILD_CONF)/lib/openpilotgcs $(DESTDIR)$(libdir)
	$(V1) $(INSTALL) $(BUILD_DIR)/openpilotgcs_$(GCS_BUILD_CONF)/share/openpilotgcs $(DESTDIR)$(datadir)
	$(V1) $(INSTALL) $(ROOT_DIR)/package/linux/openpilot.desktop $(DESTDIR)$(datadir)/applications
	$(V1) $(INSTALL) $(ROOT_DIR)/package/linux/openpilot.png $(DESTDIR)$(datadir)/pixmaps
	$(V1) rm $(DESTDIR)/$(datadir)/openpilotgcs/translations/Makefile


