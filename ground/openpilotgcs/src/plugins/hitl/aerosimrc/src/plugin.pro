!win32 {
    error("AeroSimRC plugin is only available for win32 platform")
}

include(../../../../../openpilotgcs.pri)

QT += network
QT -= gui

TEMPLATE = lib
TARGET = plugin_AeroSIMRC

RES_DIR    = $${PWD}/resources
SIM_DIR    = $$GCS_BUILD_TREE/../AeroSIM-RC
PLUGIN_DIR = $$SIM_DIR/Plugin/CopterControl
DLLDESTDIR = $$PLUGIN_DIR

HEADERS = \
    aerosimrcdatastruct.h \
    enums.h \
    plugin.h \
    qdebughandler.h \
    udpconnect.h \
    settings.h

SOURCES = \
    qdebughandler.cpp \
    plugin.cpp \
    udpconnect.cpp \
    settings.cpp

# Resemble the AeroSimRC directory structure and copy plugin files and resources
equals(copydata, 1) {

    # Windows release only
    win32:CONFIG(release, debug|release) {

        data_copy.commands += -@$(MKDIR) $$targetPath(\"$$PLUGIN_DIR\") $$addNewline()

        # resources and sample configuration
        PLUGIN_RESOURCES = \
                cc_off.tga \
                cc_off_hover.tga \
                cc_on.tga \
                cc_on_hover.tga \
                cc_plugin.ini \
                plugin.txt
        for(res, PLUGIN_RESOURCES) {
            data_copy.commands += $(COPY_FILE) $$targetPath(\"$$RES_DIR/$$res\") $$targetPath(\"$$PLUGIN_DIR/$$res\") $$addNewline()
        }

        # Qt DLLs
        QT_DLLS = \
                  QtCore4.dll \
                  QtNetwork4.dll
        for(dll, QT_DLLS) {
            data_copy.commands += $(COPY_FILE) $$targetPath(\"$$[QT_INSTALL_BINS]/$$dll\") $$targetPath(\"$$SIM_DIR/$$dll\") $$addNewline()
        }

        # MinGW DLLs
        MINGW_DLLS = \
                     libgcc_s_dw2-1.dll \
                     mingwm10.dll
        for(dll, MINGW_DLLS) {
            data_copy.commands += $(COPY_FILE) $$targetPath(\"$$[QT_INSTALL_BINS]/../../../../../mingw/bin/$$dll\") $$targetPath(\"$$SIM_DIR/$$dll\") $$addNewline()
        }

        data_copy.target = FORCE
        QMAKE_EXTRA_TARGETS += data_copy
    }
}
