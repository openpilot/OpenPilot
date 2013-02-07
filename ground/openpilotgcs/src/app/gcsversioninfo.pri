#
# This qmake file generates a header with the GCS version info string.
#
# This is a bit tricky since the script should be run always and before
# the other dependencies evaluation.
#

# Since debug_and_release option is set, we need this
!debug_and_release|build_pass {
    ROOT_DIR              = $$GCS_SOURCE_TREE/../..
    VERSION_INFO_HEADER   = $$GCS_BUILD_TREE/gcsversioninfo.h
    VERSION_INFO_SCRIPT   = $$ROOT_DIR/make/scripts/version-info.py
    VERSION_INFO_TEMPLATE = $$ROOT_DIR/make/templates/gcsversioninfotemplate.h
    VERSION_INFO_COMMAND  = python \"$$VERSION_INFO_SCRIPT\"
    UAVO_DEF_PATH         = $$ROOT_DIR/shared/uavobjectdefinition

    # Create custom version_info target which generates a header
    version_info.target   = $$VERSION_INFO_HEADER
    version_info.commands = $$VERSION_INFO_COMMAND \
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
