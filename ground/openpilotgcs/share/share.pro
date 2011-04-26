include(../openpilotgcs.pri)

TEMPLATE = subdirs
SUBDIRS = openpilotgcs/translations

DATACOLLECTIONS = dials models pfd sounds diagrams mapicons

equals(copydata, 1) {
    for(dir, DATACOLLECTIONS) {
        exists($$GCS_SOURCE_TREE/share/openpilotgcs/$$dir) {
            macx:data_copy.commands += $(COPY_DIR) $$targetPath(\"$$GCS_SOURCE_TREE/share/openpilotgcs/$$dir\") $$targetPath(\"$$GCS_DATA_PATH/\") $$addNewline()
            !macx:data_copy.commands += $(COPY_DIR) $$targetPath(\"$$GCS_SOURCE_TREE/share/openpilotgcs/$$dir\") $$targetPath(\"$$GCS_DATA_PATH/$$dir\") $$addNewline()
        }
    }

    data_copy.target = FORCE
    QMAKE_EXTRA_TARGETS += data_copy
}
