include(../../openpilotgcs.pri)

TEMPLATE = subdirs

# Some Windows packaging magic (for release build only)
equals(copydata, 1):win32:CONFIG(release, debug|release) {

    WINX86_PATH = packaging/winx86
    SVN_INFO_TEMPLATE = svninfo.tpl
    SVN_INFO_MAKEFILE = svninfo.mk
    NSIS_TEMPLATE = openpilotgcs.tpl
    NSIS_HEADER = openpilotgcs.nsh

    # Check for SubWCRev.exe executable required to get some useful SVN repository info.
    # For example, currently checked out SVN revision (highest for the working copy).
    # SubWCRev is a part of TortoiseSVN client distribution:
    # http://tortoisesvn.net/
    # SubWCRev is also available separately:
    # http://sourceforge.net/projects/tortoisesvn/files/Tools/1.6.7/SubWCRev-1.6.7.18415.msi/download

    # Default location is TortoiseSVN bin folder.
    SUBWCREV_EXE = $$targetPath(\"$$(ProgramFiles)/TortoiseSVN/bin/SubWCRev.exe\")

    exists($$SUBWCREV_EXE) {
        message("SubWCRev found: $${SUBWCREV_EXE}")
        svninfo.commands += $$SUBWCREV_EXE $$targetPath($$GCS_SOURCE_TREE)
        svninfo.commands +=   $$targetPath($$GCS_SOURCE_TREE/$$WINX86_PATH/$$SVN_INFO_TEMPLATE)
        svninfo.commands +=   $$targetPath($$GCS_BUILD_TREE/$$WINX86_PATH/$$SVN_INFO_MAKEFILE)
        svninfo.commands +=   $$addNewline()
        svninfo.commands += $$SUBWCREV_EXE $$targetPath($$GCS_SOURCE_TREE)
        svninfo.commands +=   $$targetPath($$GCS_SOURCE_TREE/$$WINX86_PATH/$$NSIS_TEMPLATE)
        svninfo.commands +=   $$targetPath($$GCS_BUILD_TREE/$$WINX86_PATH/$$NSIS_HEADER)
        svninfo.commands +=   $$addNewline()
    } else {
        message("SubWCRev not found, SVN info is not available")
        svninfo.commands += $(COPY_FILE)
        svninfo.commands +=   $$targetPath($$GCS_SOURCE_TREE/$$WINX86_PATH/$$SVN_INFO_MAKEFILE)
        svninfo.commands +=   $$targetPath($$GCS_BUILD_TREE/$$WINX86_PATH/$$SVN_INFO_MAKEFILE)
        svninfo.commands +=   $$addNewline()
        svninfo.commands += $(COPY_FILE)
        svninfo.commands +=   $$targetPath($$GCS_SOURCE_TREE/$$WINX86_PATH/$$NSIS_HEADER)
        svninfo.commands +=   $$targetPath($$GCS_BUILD_TREE/$$WINX86_PATH/$$NSIS_HEADER)
        svninfo.commands +=   $$addNewline()
    }
    svninfo.target = svninfo.dummy
    QMAKE_EXTRA_TARGETS += svninfo
    force.depends += svninfo

    # Redefine FORCE target to collect data every time
    force.target = FORCE
    QMAKE_EXTRA_TARGETS += force

    # Create installer build target - this WILL NOT run during build, run it by hand
    message("Run \"make installer\" to build Windows installer (Unicode NSIS 2.46+ required)")
    nsis.target = installer
    nsis.depends = svninfo
    nsis.commands += @$$targetPath($$GCS_SOURCE_TREE/$$WINX86_PATH/Makefile.cmd)
    QMAKE_EXTRA_TARGETS += nsis
}
