TEMPLATE = lib
TARGET = GCSGStreamer
DEFINES += GST_LIB_LIBRARY

QT += widgets

include(../../openpilotgcslibrary.pri)

# The following keeps the generated files at least somewhat separate 
# from the source files.
# TODO this section should be moved to a common place
#UI_DIR = uics
#MOC_DIR = mocs
#OBJECTS_DIR = objs

HEADERS += \
    gst_global.h \
    pipeline.h \
    pipelineevent.h \
    overlay.h \
    videowidget.h

SOURCES += \
    gst_global.cpp \
    videowidget.cpp

OTHER_FILES += \
    COPYING \
    README \
    sdlgamepad.dox \
    sdlgamepad.doc

INCLUDEPATH += \
    $(GSTREAMER_SDK_DIR)/include \
    $(GSTREAMER_SDK_DIR)/include/gstreamer-1.0 \
    $(GSTREAMER_SDK_DIR)/include/libxml2 \
    $(GSTREAMER_SDK_DIR)/include/glib-2.0 \
    $(GSTREAMER_SDK_DIR)/lib/i386-linux-gnu/glib-2.0/include

LIBS += -L$(GSTREAMER_SDK_DIR)/lib
LIBS += -lgobject-2.0 -lglib-2.0 -lgstreamer-1.0 -lgstvideo-1.0

include(copydata.pro)

#win32 {
#    # compile missing winscreencap plugin
#    include(gst-plugins-bad/gst-plugins-bad.pro)
#}

# Add the repository ppa:gstreamer-developers/ppa using Synaptic Package Manager or CLI
# > sudo add-apt-repository ppa:gstreamer-developers/ppa
# > sudo apt-get update
#
# Upgrade to latest version of the packages using Synaptic Package Manager or CLI
# > sudo apt-get install gstreamer1.0-tools gstreamer1.0-plugins-base gstreamer1.0-plugins-good gstreamer1.0-plugins-bad
# > sudo apt-get install gstreamer1.0-dev gstreamer-plugins-base1.0-dev
