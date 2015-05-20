include(../openpilotgcs.pri)

isEmpty(PROVIDER) {
    PROVIDER = OpenPilot
}

DESTDIR = $$GCS_PLUGIN_PATH/$$PROVIDER
LIBS += -L$$DESTDIR
INCLUDEPATH += $$GCS_SOURCE_TREE/src/plugins
DEPENDPATH += $$GCS_SOURCE_TREE/src/plugins

QT += widgets

# copy the plugin spec
isEmpty(TARGET) {
    error("qtcreatorplugin.pri: You must provide a TARGET")
}

PLUGINSPECS = $${_PRO_FILE_PWD_}/$${TARGET}.pluginspec
copy2build.input = PLUGINSPECS
copy2build.output = $$DESTDIR/${QMAKE_FUNC_FILE_IN_stripSrcDir}
copy2build.variable_out = PRE_TARGETDEPS
copy2build.commands = $$QMAKE_COPY ${QMAKE_FILE_IN} ${QMAKE_FILE_OUT}
copy2build.name = COPY ${QMAKE_FILE_IN}
copy2build.CONFIG += no_link
QMAKE_EXTRA_COMPILERS += copy2build

TARGET = $$qtLibraryName($$TARGET)

macx {
        QMAKE_LFLAGS_SONAME = -Wl,-install_name,@executable_path/../Plugins/$${PROVIDER}/
} else:linux-* {
    QMAKE_RPATHDIR  = $$shell_quote(\$$ORIGIN)
    QMAKE_RPATHDIR += $$shell_quote(\$$ORIGIN/$$relative_path($$GCS_LIBRARY_PATH, $$DESTDIR))
    QMAKE_RPATHDIR += $$shell_quote(\$$ORIGIN/$$relative_path($$GCS_QT_LIBRARY_PATH, $$DESTDIR))
    include(rpath.pri)
}


contains(QT_CONFIG, reduce_exports):CONFIG += hGCS_symbols

CONFIG += plugin plugin_with_soname

!macx {
    target.path = /$$GCS_LIBRARY_BASENAME/openpilotgcs/plugins/$$PROVIDER
    pluginspec.files += $${TARGET}.pluginspec
    pluginspec.path = /$$GCS_LIBRARY_BASENAME/openpilotgcs/plugins/$$PROVIDER
    INSTALLS += target pluginspec
}
