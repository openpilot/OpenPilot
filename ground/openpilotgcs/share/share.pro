include(../openpilotgcs.pri)

TEMPLATE = subdirs
SUBDIRS = openpilotgcs/translations

DATACOLLECTIONS = dials models pfd sounds diagrams mapicons

equals(copydata, 1) {
    for(dir, DATACOLLECTIONS) {
        exists($$GCS_SOURCE_TREE/share/openpilotgcs/$$dir) {
            # remove r/o and hidden attributes from .svn/* files from old build: they prevent copy over with cp -rp
            win32:data_copy.commands += -@attrib -R -S -H $$targetPath(\"$$GCS_DATA_PATH/$$dir/*\") /S /D >nul 2>&1 $$addNewline()

            macx:data_copy.commands += $(COPY_DIR) $$targetPath(\"$$GCS_SOURCE_TREE/share/openpilotgcs/$$dir\") $$targetPath(\"$$GCS_DATA_PATH/\") $$addNewline()
            !macx:data_copy.commands += $(COPY_DIR) $$targetPath(\"$$GCS_SOURCE_TREE/share/openpilotgcs/$$dir\") $$targetPath(\"$$GCS_DATA_PATH/$$dir\") $$addNewline()

            # remove r/o and hidden attributes from .svn/* files: they prevent cleaning with rm -rf
            win32:data_copy.commands += attrib -R -S -H $$targetPath(\"$$GCS_DATA_PATH/$$dir/*\") /S /D $$addNewline()
        }
    }

    data_copy.target = FORCE
    QMAKE_EXTRA_TARGETS += data_copy
}
