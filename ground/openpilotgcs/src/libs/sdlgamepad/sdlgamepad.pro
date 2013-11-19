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

TEMPLATE     = lib
TARGET      = sdlgamepad
DEFINES     += SDLGAMEPAD_LIBRARY
macx {
    # Workaround to ensure that SDL framework and associated header files are found
    INCLUDEPATH += /Library/Frameworks/SDL.framework/Headers
    SDL = -F/Library/Frameworks
    # Add SDL to CFLAGS fixes build problems on mac
    QMAKE_CFLAGS += $$SDL
    QMAKE_CXXFLAGS += $$SDL
    # Let the linker know where to find the frameworks
    LIBS += $$SDL
}

include(../../openpilotgcslibrary.pri)

SOURCES     += sdlgamepad.cpp
HEADERS     += sdlgamepad.h \
               sdlgamepad_global.h

macx:LIBS   += -framework OpenGL -framework SDL -framework Cocoa
!macx:LIBS  += -lSDL

OTHER_FILES += COPYING \
               README \
               sdlgamepad.dox \
               sdlgamepad.doc
