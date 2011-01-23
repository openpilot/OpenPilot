#
# Qmake project for UAVObjects generation.
#
# TODO: provide some dependencies (now it builds every time)
#

TEMPLATE  = subdirs

defineReplace(targetPath) {
   return($$replace(1, /, $$QMAKE_DIR_SEP))
}

defineReplace(addNewline) {
    return($$escape_expand(\\n\\t))
}

win32 {
    CONFIG(release, debug|release) {
        BUILD_SUBDIR = release/
    } else {
        BUILD_SUBDIR = debug/
    }
} else {
    BUILD_SUBDIR =
}

win32:SPEC = win32-g++
macx-g++:SPEC = macx-g++
linux-g++:SPEC = linux-g++

win32 {
    # Windows sometimes remembers working directory changed from Makefile, sometimes not.
    # That's why pushd/popd is used here - to make sure that we know current directory.

    uavobjects.commands += -$(MKDIR) $$targetPath(../../uavobject-synthetics) $$addNewline()

    uavobjects.commands += pushd $$targetPath(../../uavobject-synthetics) &&
    uavobjects.commands += $$targetPath(../ground/uavobjgenerator/$${BUILD_SUBDIR}uavobjgenerator)
    uavobjects.commands += -gcs -flight -python -matlab $$targetPath(../../shared/uavobjectdefinition)
    uavobjects.commands += $$targetPath(../..) && popd $$addNewline()

    uavobjects.commands += pushd $$targetPath(../../ground/openpilotgcs) &&
    uavobjects.commands += $(QMAKE) $$targetPath(../../../ground/openpilotgcs/)openpilotgcs.pro
    uavobjects.commands += -spec $$SPEC -r && popd $$addNewline()
}

!win32 {
    uavobjects.commands += -$(MKDIR) -p ../../uavobject-synthetics $$addNewline()

    uavobjects.commands += cd ../../uavobject-synthetics &&
    uavobjects.commands += ../ground/uavobjgenerator/uavobjgenerator
    uavobjects.commands += -gcs -flight -python -matlab ../../shared/uavobjectdefinition ../.. &&

    uavobjects.commands += cd ../ground/openpilotgcs) &&
    uavobjects.commands += $(QMAKE) ../../../ground/openpilotgcs/openpilotgcs.pro
    uavobjects.commands += -spec $$SPEC -r $$addNewline()
}

uavobjects.target = FORCE
QMAKE_EXTRA_TARGETS += uavobjects
