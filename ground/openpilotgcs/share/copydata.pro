include(../openpilotgcs.pri)

TEMPLATE = aux

DATACOLLECTIONS = cloudconfig default_configurations dials models pfd sounds diagrams mapicons stylesheets

equals(copydata, 1) {
    for(dir, DATACOLLECTIONS) {
        exists($$GCS_SOURCE_TREE/share/openpilotgcs/$$dir) {
            addCopyDirFilesTargets($$GCS_SOURCE_TREE/share/openpilotgcs/$$dir, $$GCS_DATA_PATH/$$dir)
        }
    }
}
