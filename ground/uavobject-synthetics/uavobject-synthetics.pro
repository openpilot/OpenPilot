#
# Qmake project for UAVObjects generation.
# Copyright (c) 2009-2013, The OpenPilot Team, http://www.openpilot.org
#

TEMPLATE  = subdirs

# Some handy defines
defineReplace(targetPath) {
   return($$replace(1, /, $$QMAKE_DIR_SEP))
}

defineReplace(addNewline) {
    return($$escape_expand(\\n\\t))
}

# QMAKESPEC should be defined by qmake but sometimes it is not
isEmpty(QMAKESPEC) {
    win32:SPEC = win32-g++
    macx-g++:SPEC = macx-g++
    linux-g++:SPEC = linux-g++
    linux-g++-32:SPEC = linux-g++
    linux-g++-64:SPEC = linux-g++-64
} else {
    SPEC = $$QMAKESPEC
}

# Some platform-dependent options
win32|unix {
    CONFIG(release, debug|release) {
        BUILD_CONFIG = release
    } else {
        BUILD_CONFIG = debug
    }
}

win32 {
    # Windows sometimes remembers working directory changed from Makefile, sometimes not.
    # That's why pushd/popd is used here - to make sure that we know current directory.

    uavobjects.commands += -$(MKDIR) $$targetPath(../uavobject-synthetics) $$addNewline()
    uavobjects.commands += pushd $$targetPath(../uavobject-synthetics) &&
    uavobjects.commands += $$targetPath(../uavobjgenerator/$${BUILD_CONFIG}/uavobjgenerator)
    uavobjects.commands +=   $$targetPath(../../shared/uavobjectdefinition)
    uavobjects.commands +=   $$targetPath(../..) &&
    uavobjects.commands += popd $$addNewline()

    uavobjects.commands += -$(MKDIR) $$targetPath(../openpilotgcs) $$addNewline()
    uavobjects.commands += pushd $$targetPath(../openpilotgcs) &&
    uavobjects.commands += $(QMAKE) -spec $$SPEC CONFIG+=$${BUILD_CONFIG} -r
    uavobjects.commands +=   $$targetPath(../../ground/openpilotgcs/)openpilotgcs.pro &&
    uavobjects.commands += popd $$addNewline()
}

!win32 {
    uavobjects.commands += $(MKDIR) -p ../uavobject-synthetics $$addNewline()
    uavobjects.commands += cd ../uavobject-synthetics &&
    uavobjects.commands += ../uavobjgenerator/uavobjgenerator
    uavobjects.commands +=   ../../shared/uavobjectdefinition ../.. &&

    uavobjects.commands += $(MKDIR) -p ../openpilotgcs $$addNewline()
    uavobjects.commands += cd ../openpilotgcs &&
    uavobjects.commands += $(QMAKE) ../../ground/openpilotgcs/openpilotgcs.pro
    uavobjects.commands +=   -spec $$SPEC CONFIG+=$${BUILD_CONFIG} -r $$addNewline()
}

uavobjects.target = FORCE
QMAKE_EXTRA_TARGETS += uavobjects
