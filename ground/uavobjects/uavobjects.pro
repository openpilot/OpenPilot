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

win32:MKDIR=$(MKDIR)
!win32:MKDIR=$(MKDIR) -p

uavobjects.target = FORCE
uavobjects.commands += -$${MKDIR} $$targetPath(../../uavobject-synthetics) $$addNewline()
uavobjects.commands += cd $$targetPath(../../uavobject-synthetics) && 
uavobjects.commands += $$targetPath(../ground/uavobjgenerator/$${BUILD_SUBDIR}uavobjgenerator)
uavobjects.commands += -gcs ../../shared/uavobjectdefinition ../.. $$addNewline()
QMAKE_EXTRA_TARGETS += uavobjects
