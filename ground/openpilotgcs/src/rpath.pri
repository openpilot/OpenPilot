linux {
    # HACK! Do the rpath by hand since it's not possible to use ORIGIN in QMAKE_RPATHDIR before Qt 5.4
    # \'\$$ORIGIN\'  expands to $ORIGIN (after qmake and make), it does NOT read a qmake var
    GCS_PLUGIN_RPATH = $$join(QMAKE_RPATHDIR, ":")
 
    QMAKE_LFLAGS += -Wl,-z,origin,-rpath,$${GCS_PLUGIN_RPATH}
    QMAKE_RPATHDIR =
}
