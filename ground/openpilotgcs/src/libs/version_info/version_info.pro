TEMPLATE = lib
TARGET = VersionInfo

include(../../openpilotgcslibrary.pri)
include(../../python.pri)

HEADERS = version_info.h
SOURCES = version_info.cpp

#
# This qmake code generates VersionInfo class implementation.
#
# This is a bit tricky since the script should be run always and before
# the other dependencies evaluation.
#

# Since debug_and_release option is set, we need this
!debug_and_release|build_pass {
    # Define other variables
    VERSION_INFO_DIR      = $$GCS_BUILD_TREE/../openpilotgcs-synthetics
    VERSION_INFO_SCRIPT   = $$ROOT_DIR/make/scripts/version-info.py
    VERSION_INFO_COMMAND  = $$PYTHON \"$$VERSION_INFO_SCRIPT\"
    VERSION_INFO_TEMPLATE = $$GCS_SOURCE_TREE/src/libs/version_info/version_info.cpp.template
    VERSION_INFO_FILE     = $$VERSION_INFO_DIR/version_info.cpp
    UAVO_DEF_PATH         = $$ROOT_DIR/shared/uavobjectdefinition

    # Create custom version_info target which generates a real file
    version_info.target   = $$VERSION_INFO_FILE
    version_info.commands = -$(MKDIR) $$targetPath($$VERSION_INFO_DIR) $$addNewline()
    version_info.commands += $$VERSION_INFO_COMMAND \
                                    --path=\"$$GCS_SOURCE_TREE\" \
                                    --template=\"$$VERSION_INFO_TEMPLATE\" \
                                    --uavodir=\"$$UAVO_DEF_PATH\" \
                                    --outfile=\"$$VERSION_INFO_FILE\"
    version_info.depends = FORCE
    QMAKE_EXTRA_TARGETS += version_info

    # Hook version_info target in between qmake's Makefile update and
    # the actual project target
    version_info_hook.depends = version_info
    debug_and_release {
        CONFIG(debug,debug|release):version_info_hook.target = Makefile.Debug
        CONFIG(release,debug|release):version_info_hook.target = Makefile.Release
    } else {
        version_info_hook.target = Makefile
    }
    QMAKE_EXTRA_TARGETS += version_info_hook
}
