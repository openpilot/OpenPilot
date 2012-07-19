!win32 { error("AeroSimRC plugin is only available for win32 platform") }

QT += network
QT -= gui
TARGET = plugin_AeroSIMRC
TEMPLATE = lib

# Don't depend on MSVRT*.dll
win32-msvc* {
    QMAKE_CXXFLAGS_RELEASE -= -MD
    QMAKE_CXXFLAGS_MT_DLL += -MT
}

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

#DLLDESTDIR = ../CopterControl
