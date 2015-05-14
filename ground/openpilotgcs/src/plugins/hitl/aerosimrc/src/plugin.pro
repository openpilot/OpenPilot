!win32 {
    error("AeroSimRC plugin is only available for win32 platform")
}

include(../../../../../openpilotgcs.pri)

QT += network
QT -= gui

TEMPLATE = lib
TARGET = plugin_AeroSIMRC

RES_DIR    = $${PWD}/resources
SIM_DIR    = $$GCS_BUILD_TREE/misc/AeroSIM-RC
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

        # resources and sample configuration
        PLUGIN_RESOURCES = \
                cc_off.tga \
                cc_off_hover.tga \
                cc_on.tga \
                cc_on_hover.tga \
                cc_plugin.ini \
                plugin.txt
        for(res, PLUGIN_RESOURCES) {
            addCopyFileTarget($${res},$${RES_DIR},$${PLUGIN_DIR})
        }

        # Qt DLLs
        QT_DLLS = \
                  Qt5Core.dll \
                  Qt5Network.dll
        for(dll, QT_DLLS) {
            addCopyFileTarget($${dll},$$[QT_INSTALL_BINS],$${SIM_DIR})
        }

        # MinGW DLLs
        #MINGW_DLLS = \
        #             libgcc_s_dw2-1.dll \
        #             mingwm10.dll
        #for(dll, MINGW_DLLS) {
        #    addCopyFileTarget($${dll},$$(QTMINGW),$${SIM_DIR})
        #}
    }
}
