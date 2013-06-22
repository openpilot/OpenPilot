#
# MacOSX-specific packaging script
#

ifndef OPENPILOT_IS_COOL
    $(error Top level Makefile must be used to build this target)
endif

FW_DIR := $(PACKAGE_DIR)/firmware

.PHONY: package
package:
	( \
	  ROOT_DIR="$(ROOT_DIR)" \
	  BUILD_DIR="$(BUILD_DIR)" \
	  PACKAGE_LBL="$(PACKAGE_LBL)" \
	  PACKAGE_DIR="$(PACKAGE_DIR)" \
	  PACKAGE_NAME="$(PACKAGE_NAME)" \
	  PACKAGE_SEP="$(PACKAGE_SEP)" \
	  FW_DIR="$(FW_DIR)" \
	  "$(ROOT_DIR)/package/osx/package" \
	)
