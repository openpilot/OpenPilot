DEFINES += HAVE_CONFIG_H
#DEFINES += GST_PLUGIN_BUILD_STATIC

HEADERS += \
	gst-plugins-bad/sys/winks/gstksclock.h \
	gst-plugins-bad/sys/winks/gstksvideodevice.h \
	gst-plugins-bad/sys/winks/gstksvideosrc.h \
	gst-plugins-bad/sys/winks/kshelpers.h \
	gst-plugins-bad/sys/winks/ksvideohelpers.h
	
SOURCES += \
	gst-plugins-bad/sys/winks/gstksclock.c \
	gst-plugins-bad/sys/winks/gstksvideodevice.c \
	gst-plugins-bad/sys/winks/gstksvideosrc.c \
	gst-plugins-bad/sys/winks/kshelpers.c \
	gst-plugins-bad/sys/winks/ksvideohelpers.c

INCLUDEPATH += gst-plugins-bad/win32/common

LIBS += -L$(GSTREAMER_SDK_DIR)/lib
LIBS += -lgstbase-1.0

# winks libs
#LIBS += -L$(MINGW_DIR)/i686-w64-mingw32/lib
LIBS += -ldxguid -lole32 -luuid -lstrmiids -lksuser -lsetupapi
