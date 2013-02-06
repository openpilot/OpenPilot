#
# This file is part of SDLGamepad.
#
# SDLGamepad is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# SDLGamepad is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
#
# Manuel Blanquett
# mail.nalla@gmail.com
#

TEMPLATE    = lib
TARGET      = GST_lib
DEFINES     += GST_LIB_LIBRARY

! include(../../openpilotgcslibrary.pri) {
    error( Couldn't find the openpilotgcslibrary.pri file! )
}

# The following keeps the generated files at least somewhat separate 
# from the source files.
# TODO this section should be moved to a common place
#UI_DIR = uics
#MOC_DIR = mocs
#OBJECTS_DIR = objs

HEADERS += gst_global.h \
	pipeline.h \
	pipelineevent.h \
	overlay.h \
    videowidget.h
	
SOURCES += gst_global.cpp \
	videowidget.cpp

OTHER_FILES += COPYING \
               README \
               sdlgamepad.dox \
               sdlgamepad.doc

INCLUDEPATH += \
    $(GSTREAMER_SDK_DIR)/include \
	$(GSTREAMER_SDK_DIR)/include/gstreamer-0.10 \
    $(GSTREAMER_SDK_DIR)/include/libxml2 \
	$(GSTREAMER_SDK_DIR)/include/glib-2.0 \
	$(GSTREAMER_SDK_DIR)/lib/glib-2.0/include

LIBS += -L$(GSTREAMER_SDK_DIR)/lib
LIBS += -lgobject-2.0 -lglib-2.0 -lgstreamer-0.10 -lgstinterfaces-0.10

#win32 {
#	# compile missing winscreencap plugin (should be removed once it is available with the GStreamer SDK)
#	include(gst-plugins-bad/gst-plugins-bad.pro)
#}