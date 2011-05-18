include(../../openpilotgcs.pri)

TEMPLATE = subdirs

# Some Windows packaging magic (for release build only)
equals(copydata, 1):win32:CONFIG(release, debug|release) {

    # We need this Windows macro since building under Unix-like shell the top level
    # targetPath macro will use forward slashes which don't work for such Windows
    # commands like pushd, etc. But note that we still use targetPath for $(COPY_FILE)
    # parameters because this command is different under native Windows and Unix-like
    # build environments.
    defineReplace(winTargetPath) {
        return($$replace(1, /, \\))
    }

    # Some file locations
    WINX86_PATH     = packaging/winx86
    NSIS_HEADER     = openpilotgcs.nsh
    HEADER_MAKER    = make_header.cmd
    INSTALLER_MAKER = make_installer.cmd

    # copy defaults first (will be used if no git available)
    git.commands += @echo Copying default version info... $$addNewline()
    git.commands += $(COPY_FILE)
    git.commands +=   $$targetPath($$GCS_SOURCE_TREE/$$WINX86_PATH/$$NSIS_HEADER)
    git.commands +=   $$targetPath($$GCS_BUILD_TREE/$$WINX86_PATH/$$NSIS_HEADER)
    git.commands +=   $$addNewline()

    # extract repository info if command line git is available
    git.commands += $$winTargetPath($$GCS_SOURCE_TREE/$$WINX86_PATH/$$HEADER_MAKER)
    git.commands +=   $$winTargetPath($$GCS_SOURCE_TREE)
    git.commands +=   $$winTargetPath($$GCS_BUILD_TREE/$$WINX86_PATH/$$NSIS_HEADER)
    git.commands +=   $$addNewline()

    git.target = git.dummy
    QMAKE_EXTRA_TARGETS += git
    force.depends += git

    # Redefine FORCE target to collect data every time
    force.target = FORCE
    QMAKE_EXTRA_TARGETS += force

    # Create installer build target - this WILL NOT run during build, run it by hand
    message("Run \"make gcs_installer\" in $$GCS_BUILD_TREE/$$WINX86_PATH to build Windows installer (Unicode NSIS 2.46+ required)")
    nsis.target = gcs_installer
    nsis.depends = git
    nsis.commands += @$$winTargetPath($$GCS_SOURCE_TREE/$$WINX86_PATH/$$INSTALLER_MAKER)
    QMAKE_EXTRA_TARGETS += nsis
}
