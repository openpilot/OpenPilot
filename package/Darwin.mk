#
# MacOSX-specific packaging script
#

ifndef OPENPILOT_IS_COOL
    $(error Top level Makefile must be used to build this target)
endif

.PHONY: package
package: openpilotgcs uavobjects_matlab | $(PACKAGE_DIR)
ifneq ($(GCS_BUILD_CONF),release)
	# We can only package release builds
	$(error Packaging is currently supported for release builds only)
endif
	( \
	  ROOT_DIR="$(ROOT_DIR)" \
	  BUILD_DIR="$(BUILD_DIR)" \
	  GCS_BIG_NAME="$(GCS_BIG_NAME)" \
	  GCS_SMALL_NAME="$(GCS_SMALL_NAME)" \
	  PACKAGE_LBL="$(PACKAGE_LBL)" \
	  PACKAGE_DIR="$(PACKAGE_DIR)" \
	  PACKAGE_NAME="$(PACKAGE_NAME)" \
	  PACKAGE_SEP="$(PACKAGE_SEP)" \
	  "$(ROOT_DIR)/package/osx/package" \
	)
