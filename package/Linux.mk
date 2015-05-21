#
# Linux-specific packaging script
#

ifndef OPENPILOT_IS_COOL
    $(error Top level Makefile must be used to build this target)
endif

# Are we using a debian based distro?
ifneq ($(shell which dpkg 2> /dev/null),)

DEB_DIST             := unstable
# Instead of RELEASE-15.01-RC1 debian wants 15.01~RC1
UPSTREAM_VER         := $(subst -,~,$(subst RELEASE-,,$(PACKAGE_LBL)))
DEB_REV              := 1
ifeq ($(DEB_DIST), trusty)
DEB_REV              := $(DEB_REV)$(DEB_DIST)1
endif
DEB_NAME             := $(OP_SMALL_NAME)
DEB_ORIG_SRC         := $(PACKAGE_DIR)/$(DEB_NAME)_$(UPSTREAM_VER).orig.tar.gz
DEB_PACKAGE_DIR      := $(PACKAGE_DIR)/$(DEB_NAME)-$(UPSTREAM_VER)
DEB_ARCH             := $(shell dpkg --print-architecture)
DEB_PACKAGE_NAME     := $(DEB_NAME)_$(UPSTREAM_VER)-$(DEB_REV)_$(DEB_ARCH)
DEB_DIR              := package/linux/debian

SED_DATE_STRG         = $(shell date -R)
SED_SCRIPT            = sed -i -e ' \
			s/<VERSION>/$(UPSTREAM_VER)-$(DEB_REV)/g; \
			s/<DATE>/$(SED_DATE_STRG)/g; \
			s/<DIST>/$(DEB_DIST)/g; \
			s/<NAME>/$(DEB_NAME)/g; \
			'

# Ubuntu 14.04 (Trusty Tahr) has different names for the qml-modules
TRUSTY_DEPS_SED      := s/qml-module-qtquick-controls/qtdeclarative5-controls-plugin/g; \
	s/qml-module-qtquick-dialogs/qtdeclarative5-dialogs-plugin/g; \
	s/qml-module-qtquick-localstorage/qtdeclarative5-localstorage-plugin/g; \
	s/qml-module-qtquick-particles2/qtdeclarative5-particles-plugin/g; \
	s/qml-module-qtquick2/qtdeclarative5-qtquick2-plugin/g; \
	s/qml-module-qtquick-window2/qtdeclarative5-window-plugin/g; \
	s/qml-module-qtquick-xmllistmodel/qtdeclarative5-xmllistmodel-plugin/g;

# Leave off Qt and ARM compiler dependencies if calling package target under the assumption that
# OP is providing them or the user already has them installed because OP is already built.
PACKAGE_DEPS_SED     := s/python.*/python/;s/{misc:Depends}.*/{misc:Depends}/;

.PHONY: package
package: debian
	@$(ECHO) "Building Linux package, please wait..."
	$(V1) sed -i -e "$(PACKAGE_DEPS_SED)" debian/control
	$(V1) dpkg-buildpackage -b -us -uc -nc
	$(V1) mv $(ROOT_DIR)/../$(DEB_PACKAGE_NAME).deb $(BUILD_DIR)
	$(V1) mv $(ROOT_DIR)/../$(DEB_PACKAGE_NAME).changes $(BUILD_DIR)
	$(V1) rm -r debian

.PHONY: debian
debian: $(DEB_DIR)
	$(V1) rm -rf debian
	$(V1) cp -r $(DEB_DIR) debian
	$(V1) cp -T package/linux/45-openpilot-permissions.rules debian/$(DEB_NAME).udev
	$(V1) $(SED_SCRIPT) debian/changelog debian/control
ifeq ($(DEB_DIST), trusty)
	$(V1) sed -i -e "$(TRUSTY_DEPS_SED)" debian/control
endif

.PHONY: package_src
package_src:  $(DEB_ORIG_SRC_NAME) $(DEB_PACKAGE_DIR)
	$(V1) cd $(DEB_PACKAGE_DIR) && dpkg-buildpackage -S -us -uc

$(DEB_ORIG_SRC): $(DIST_NAME).gz | $(PACKAGE_DIR)
	$(V1) cp $(DIST_NAME).gz $(DEB_ORIG_SRC)

$(DEB_PACKAGE_DIR): $(DEB_ORIG_SRC) debian | $(PACKAGE_DIR)
	$(V1) tar -xf $(DEB_ORIG_SRC) -C $(PACKAGE_DIR)
	$(V1) mv debian $(PACKAGE_DIR)/$(PACKAGE_NAME)
	$(V1) rm -rf $(DEB_PACKAGE_DIR) && mv $(PACKAGE_DIR)/$(PACKAGE_NAME) $(DEB_PACKAGE_DIR)

endif # Debian based distro?
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
	$(V1) $(INSTALL) $(BUILD_DIR)/$(GCS_SMALL_NAME)_$(GCS_BUILD_CONF)/bin/$(GCS_SMALL_NAME) $(DESTDIR)$(bindir)
	$(V1) $(INSTALL) $(BUILD_DIR)/$(GCS_SMALL_NAME)_$(GCS_BUILD_CONF)/lib/$(GCS_SMALL_NAME) $(DESTDIR)$(libdir)
	$(V1) $(INSTALL) $(BUILD_DIR)/$(GCS_SMALL_NAME)_$(GCS_BUILD_CONF)/share/$(GCS_SMALL_NAME) $(DESTDIR)$(datadir)
	$(V1) $(INSTALL) -T $(ROOT_DIR)/package/linux/openpilot.desktop $(DESTDIR)$(datadir)/applications/$(OP_SMALL_NAME).desktop
	$(V1) sed -i -e 's/openpilotgcs/$(GCS_SMALL_NAME)/g;s/OpenPilot GCS/$(GCS_BIG_NAME)/g' $(DESTDIR)$(datadir)/applications/$(OP_SMALL_NAME).desktop
	$(V1) $(INSTALL) -T $(ROOT_DIR)/package/linux/openpilot.png $(DESTDIR)$(datadir)/pixmaps/$(OP_SMALL_NAME).png


