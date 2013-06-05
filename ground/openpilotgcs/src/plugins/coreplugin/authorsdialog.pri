include(../../python.pri)

#
# This qmake code generates AuthorsModel.qml using CREDITS.txt.
#
# This is a bit tricky since the script should be run always and before
# the other dependencies evaluation.
#

# Since debug_and_release option is set, we need this
!debug_and_release|build_pass {

    # Define other variables
    AUTHORS_SCRIPT   = $$GCS_SOURCE_TREE/src/plugins/coreplugin/authorsdialog.py
    AUTHORS_COMMAND  = $$PYTHON \"$$AUTHORS_SCRIPT\"
    AUTHORS_SOURCE   = $$ROOT_DIR/CREDITS.txt
    AUTHORS_TEMPLATE = $$GCS_SOURCE_TREE/src/plugins/coreplugin/qml/AuthorsModel.qml.template
    AUTHORS_DIR      = $$GCS_BUILD_TREE/../openpilotgcs-synthetics
    AUTHORS_FILE     = $$AUTHORS_DIR/AuthorsModel.qml

    # Create custom authors target which generates a real file
    authors.target   = $$AUTHORS_FILE
    authors.commands = -$(MKDIR) $$targetPath($$AUTHORS_DIR) $$addNewline()
    authors.commands += $$AUTHORS_COMMAND \
                            --infile=\"$$AUTHORS_SOURCE\" \
                            --template=\"$$AUTHORS_TEMPLATE\" \
                            --outfile=\"$$AUTHORS_FILE\"
    authors.depends = FORCE
    QMAKE_EXTRA_TARGETS += authors

    # Hook authors target in between qmake's Makefile update and
    # the actual project target
    authors_hook.depends = authors
    debug_and_release {
        CONFIG(debug,debug|release):authors_hook.target = Makefile.Debug
        CONFIG(release,debug|release):authors_hook.target = Makefile.Release
    } else {
        authors_hook.target = Makefile
    }
    QMAKE_EXTRA_TARGETS += authors_hook
}
