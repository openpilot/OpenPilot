TEMPLATE = lib
TARGET = GCSGStreamer
DEFINES += GST_LIB_LIBRARY
DEFINES += GST_PLUGIN_BUILD_STATIC

QT += widgets

include(../../openpilotgcslibrary.pri)
include(../utils/utils.pri)

HEADERS += \
    gst_global.h \
    pipeline.h \
    pipelineevent.h \
    overlay.h \
    videowidget.h

SOURCES += \
    gst_global.cpp \
    pipeline.cpp \
    videowidget.cpp

INCLUDEPATH += \
    $(GSTREAMER_SDK_DIR)/include \
    $(GSTREAMER_SDK_DIR)/include/gstreamer-1.0 \
    $(GSTREAMER_SDK_DIR)/include/libxml2 \
    $(GSTREAMER_SDK_DIR)/include/glib-2.0

linux {
    INCLUDEPATH += $(GSTREAMER_SDK_DIR)/lib/i386-linux-gnu/glib-2.0/include
}

win32 {
    INCLUDEPATH += $(GSTREAMER_SDK_DIR)/lib/glib-2.0/include
}

LIBS += -L$(GSTREAMER_SDK_DIR)/lib
LIBS += -lgobject-2.0 -lglib-2.0 -lgstreamer-1.0 -lgstvideo-1.0

win32 {
    #include(gst-plugins-bad/gst-plugins-bad.pro)
}

include(copydata.pro)

# Linux
# Add the repository ppa:gstreamer-developers/ppa using Synaptic Package Manager or CLI
# > sudo add-apt-repository ppa:gstreamer-developers/ppa
# > sudo apt-get update
#
# Upgrade to latest version of the packages using Synaptic Package Manager or CLI
# > sudo apt-get install gstreamer1.0-tools gstreamer1.0-plugins-base gstreamer1.0-plugins-good gstreamer1.0-plugins-bad
# > sudo apt-get install gstreamer1.0-dev gstreamer-plugins-base1.0-dev
