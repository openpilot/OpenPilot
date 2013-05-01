#
# This qmake file generates a header with the GCS version info string.
#
# This is a bit tricky since the script should be run always and before
# the other dependencies evaluation.
#

# We use python to extract git version info, but it may be installed locally.
# This expected python version should be kept in sync with make/tools.mk.
PYTHON_DIR = python-2.7.4

# Since debug_and_release option is set, we need this
!debug_and_release|build_pass {

    ROOT_DIR = $$GCS_SOURCE_TREE/../..

    # Search the python using environment override first
    OPENPILOT_TOOLS_DIR = $$(OPENPILOT_TOOLS_DIR)
    !isEmpty(OPENPILOT_TOOLS_DIR):exists($$OPENPILOT_TOOLS_DIR/$$PYTHON_DIR/python*) {
        PYTHON = \"$$OPENPILOT_TOOLS_DIR/$$PYTHON_DIR/python\"
    } else {
        # If not found, search the predefined tools path
        exists($$ROOT_DIR/tools/$$PYTHON_DIR/python*) {
            PYTHON = \"$$ROOT_DIR/tools/$$PYTHON_DIR/python\"
        } else {
            # not found, hope it's in the path...
            PYTHON = \"python\"
        }
    }
    PYTHON = $$replace(PYTHON, \\\\, /)
    message(Using python interpreter: $$PYTHON)

    # Define other variables
    VERSION_INFO_DIR      = $$GCS_BUILD_TREE/../openpilotgcs-synthetics
    VERSION_INFO_HEADER   = $$VERSION_INFO_DIR/gcs_version_info.h
    VERSION_INFO_SCRIPT   = $$ROOT_DIR/make/scripts/version-info.py
    VERSION_INFO_TEMPLATE = $$ROOT_DIR/make/templates/gcs_version_info.h.template
    VERSION_INFO_COMMAND  = $$PYTHON \"$$VERSION_INFO_SCRIPT\"
    UAVO_DEF_PATH         = $$ROOT_DIR/shared/uavobjectdefinition

    # Create custom version_info target which generates a header
    version_info.target   = $$VERSION_INFO_HEADER
    version_info.commands = -$(MKDIR) $$targetPath($$VERSION_INFO_DIR) $$addNewline()
    version_info.commands += $$VERSION_INFO_COMMAND \
                                    --path=\"$$GCS_SOURCE_TREE\" \
                                    --template=\"$$VERSION_INFO_TEMPLATE\" \
                                    --uavodir=\"$$UAVO_DEF_PATH\" \
                                    --outfile=\"$$VERSION_INFO_HEADER\"
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
