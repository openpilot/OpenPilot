######################################################################
# Install paths
######################################################################

VER_MAJ      = 5
VER_MIN      = 2
VER_PAT      = 0
VERSION      = $${VER_MAJ}.$${VER_MIN}.$${VER_PAT}

######################################################################
# qmake internal options
######################################################################

CONFIG           += qt     # Also for Qtopia Core!
CONFIG           += warn_on
CONFIG           += thread

######################################################################
# release/debug mode
# If you want to build both DEBUG_SUFFIX and RELEASE_SUFFIX
# have to differ to avoid, that they overwrite each other.
######################################################################

VVERSION = $$[QT_VERSION]
isEmpty(VVERSION) {

    # Qt 3
    CONFIG           += release     # release/debug
}
else {
    # Qt 4
    win32 {
        # On Windows you can't mix release and debug libraries.
        # The designer is built in release mode. If you like to use it
        # you need a release version. For your own application development you
        # might need a debug version. 
        # Enable debug_and_release + build_all if you want to build both.

        CONFIG           += release     # release/debug/debug_and_release
        #CONFIG           += release_and_release
        #CONFIG           += build_all
    }
    else {
        CONFIG           += release     # release/debug
    }
}

######################################################################
# If you want to have different names for the debug and release 
# versions you can add a suffix rule below.
######################################################################

DEBUG_SUFFIX        = 
RELEASE_SUFFIX      = 

win32 {
    DEBUG_SUFFIX      = d
}

######################################################################
# Build the static/shared libraries.
# If QwtDll is enabled, a shared library is built, otherwise
# it will be a static library.
######################################################################

CONFIG           += QwtDll

######################################################################
# QwtPlot enables all classes, that are needed to use the QwtPlot 
# widget. 
######################################################################

CONFIG       += QwtPlot

######################################################################
# QwtWidgets enables all classes, that are needed to use the all other
# widgets (sliders, dials, ...), beside QwtPlot. 
######################################################################

CONFIG     += QwtWidgets

######################################################################
# If you want to build the Qwt designer plugin, 
# enable the line below.
# Otherwise you have to build it from the designer directory.
######################################################################

#CONFIG     += QwtDesigner

