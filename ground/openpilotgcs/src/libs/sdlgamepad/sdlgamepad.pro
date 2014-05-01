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

TEMPLATE = lib
TARGET = sdlgamepad
DEFINES += SDLGAMEPAD_LIBRARY

include(../../openpilotgcslibrary.pri)

macx {
    # Ensures that SDL framework and header files are found when compiled with Qt5.2.1
    INCLUDEPATH += /Library/Frameworks/SDL.framework/Headers
    SDL = -F/Library/Frameworks
    # Add SDL to CFLAGS fixes build problems on mac
    QMAKE_CFLAGS += $$SDL
    QMAKE_CXXFLAGS += $$SDL
    # Let the linker know where to find the frameworks
    LIBS += $$SDL
    LIBS += -framework OpenGL -framework SDL -framework Cocoa
}

win32 {
    INCLUDEPATH += $(SDL_DIR)/include
    LIBS += -L$(SDL_DIR)/lib
}

!mac:LIBS += -lSDL

SOURCES += \
    sdlgamepad.cpp

HEADERS += \
    sdlgamepad.h \
    sdlgamepad_global.h

OTHER_FILES += \
	COPYING \
    README \
    sdlgamepad.dox \
    sdlgamepad.doc

include(copydata.pro)
