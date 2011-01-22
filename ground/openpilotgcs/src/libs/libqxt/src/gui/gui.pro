CLEAN_TARGET     = QxtGui
DEFINES         += BUILD_QXT_GUI
QT               = core gui
QXT              = core
CONVENIENCE     += $$CLEAN_TARGET

include(gui.pri)
#include(../qxtbase.pri)

contains(DEFINES,HAVE_XRANDR){
    !win32:LIBS += -lXrandr
}

win32:LIBS      += -luser32
macx:LIBS       += -framework Carbon
# Debian and derivatives pass --no-undefined to the linker, which
# means that each library must explicitly link to all dependencies 
# than assuming that the application or another library will bring
# in the necessary symbols at run time.
contains(QMAKE_LFLAGS, "-Wl,--no-undefined"):LIBS += $${QMAKE_LIBS_X11}
