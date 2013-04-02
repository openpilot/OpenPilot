include(../openpilotgcs.pri)

TEMPLATE = subdirs
SUBDIRS = openpilotgcs/translations

DATACOLLECTIONS = dials models pfd sounds diagrams mapicons stylesheets default_configurations

equals(copydata, 1) {
    for(dir, DATACOLLECTIONS) {
        exists($$GCS_SOURCE_TREE/share/openpilotgcs/$$dir) {
            # Qt make macros (CHK_DIR_EXISTS, COPY_DIR, etc) have different syntax. They cannot be used
            # reliably to copy subdirectories in two different Windows environments (bash and cmd/QtCreator).
            # So undocumented QMAKE_SH variable is used to find out the real environment.
            !isEmpty(QMAKE_SH) {
                # sh environment (including Windows bash)
                data_copy.commands += $(MKDIR) $$targetPath(\"$$GCS_DATA_PATH/$$dir\") $$addNewline()
                data_copy.commands += $(COPY_DIR) $$targetPath(\"$$GCS_SOURCE_TREE/share/openpilotgcs/$$dir\") $$targetPath(\"$$GCS_DATA_PATH/\") $$addNewline()
            } else {
                # native Windows cmd environment
                data_copy.commands += $(COPY_DIR) $$targetPath(\"$$GCS_SOURCE_TREE/share/openpilotgcs/$$dir\") $$targetPath(\"$$GCS_DATA_PATH/$$dir\") $$addNewline()
            }
        }
    }
    data_copy.target = FORCE
    QMAKE_EXTRA_TARGETS += data_copy
}
