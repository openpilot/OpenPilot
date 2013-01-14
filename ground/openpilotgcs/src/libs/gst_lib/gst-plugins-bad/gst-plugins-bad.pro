DEFINES     += HAVE_CONFIG_H

HEADERS += gst-plugins-bad/sys/winscreencap/gstwinscreencap.h \
	gst-plugins-bad/sys/winscreencap/gstdx9screencapsrc.h \
	gst-plugins-bad/sys/winscreencap/gstgdiscreencapsrc.h
	
SOURCES += gst-plugins-bad/sys/winscreencap/gstwinscreencap.c \
	gst-plugins-bad/sys/winscreencap/gstdx9screencapsrc.c \
	gst-plugins-bad/sys/winscreencap/gstgdiscreencapsrc.c

INCLUDEPATH += gst-plugins-bad/win32/common

LIBS += -lgstbase-0.10
LIBS += -L"d:/OpenPilotDev/QtSDK/mingw/lib" -ld3d9 -lgdi32