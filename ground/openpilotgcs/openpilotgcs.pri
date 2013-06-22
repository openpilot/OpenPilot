defineReplace(cleanPath) {
    win32:1 ~= s|\\\\|/|g
    contains(1, ^/.*):pfx = /
    else:pfx =
    segs = $$split(1, /)
    out =
    for(seg, segs) {
        equals(seg, ..):out = $$member(out, 0, -2)
        else:!equals(seg, .):out += $$seg
    }
    return($$join(out, /, $$pfx))
}

defineReplace(targetPath) {
    return($$replace(1, /, $$QMAKE_DIR_SEP))
}

defineReplace(addNewline) { 
    return($$escape_expand(\\n\\t))
}

defineReplace(qtLibraryName) {
   unset(LIBRARY_NAME)
   LIBRARY_NAME = $$1
   CONFIG(debug, debug|release) {
      !debug_and_release|build_pass {
          mac:RET = $$member(LIBRARY_NAME, 0)_debug
              else:win32:RET = $$member(LIBRARY_NAME, 0)d
      }
   }
   isEmpty(RET):RET = $$LIBRARY_NAME
   return($$RET)
}

# For use in custom compilers which just copy files
win32:i_flag = i
defineReplace(stripSrcDir) {
    win32 {
        !contains(1, ^.:.*):1 = $$OUT_PWD/$$1
    } else {
        !contains(1, ^/.*):1 = $$OUT_PWD/$$1
    }
    out = $$cleanPath($$1)
    out ~= s|^$$re_escape($$PWD/)||$$i_flag
    return($$out)
}

isEmpty(TEST):CONFIG(debug, debug|release) {
    !debug_and_release|build_pass {
        TEST = 1
    }
}

isEmpty(GCS_LIBRARY_BASENAME) {
    GCS_LIBRARY_BASENAME = lib
}

DEFINES += GCS_LIBRARY_BASENAME=\\\"$$GCS_LIBRARY_BASENAME\\\"

equals(TEST, 1) {
    QT +=testlib
    DEFINES += WITH_TESTS
}

#ideally, we would want a qmake.conf patch, but this does the trick...
win32:!isEmpty(QMAKE_SH):QMAKE_COPY_DIR = cp -r -f

GCS_SOURCE_TREE = $$PWD
isEmpty(GCS_BUILD_TREE) {
    sub_dir = $$_PRO_FILE_PWD_
    sub_dir ~= s,^$$re_escape($$PWD),,
    GCS_BUILD_TREE = $$cleanPath($$OUT_PWD)
    GCS_BUILD_TREE ~= s,$$re_escape($$sub_dir)$,,
}
GCS_APP_PATH = $$GCS_BUILD_TREE/bin
macx {
    GCS_APP_TARGET   = "OpenPilot GCS"
    GCS_LIBRARY_PATH = $$GCS_APP_PATH/$${GCS_APP_TARGET}.app/Contents/Plugins
    GCS_PLUGIN_PATH  = $$GCS_LIBRARY_PATH
    GCS_LIBEXEC_PATH = $$GCS_APP_PATH/$${GCS_APP_TARGET}.app/Contents/Resources
    GCS_DATA_PATH    = $$GCS_APP_PATH/$${GCS_APP_TARGET}.app/Contents/Resources
    GCS_DATA_BASENAME = Resources
    GCS_DOC_PATH     = $$GCS_DATA_PATH/doc
    copydata = 1
} else {
    win32 {
        contains(TEMPLATE, vc.*)|contains(TEMPLATE_PREFIX, vc):vcproj = 1
        GCS_APP_TARGET   = openpilotgcs
    } else {
        GCS_APP_WRAPPER  = openpilotgcs
        GCS_APP_TARGET   = openpilotgcs.bin
    }
    GCS_LIBRARY_PATH = $$GCS_BUILD_TREE/$$GCS_LIBRARY_BASENAME/openpilotgcs
    GCS_PLUGIN_PATH  = $$GCS_LIBRARY_PATH/plugins
    GCS_LIBEXEC_PATH = $$GCS_APP_PATH # FIXME
    GCS_DATA_PATH    = $$GCS_BUILD_TREE/share/openpilotgcs
    GCS_DATA_BASENAME = share/openpilotgcs
    GCS_DOC_PATH     = $$GCS_BUILD_TREE/share/doc
    !isEqual(GCS_SOURCE_TREE, $$GCS_BUILD_TREE):copydata = 1
}


DEFINES += GCS_DATA_BASENAME=\\\"$$GCS_DATA_BASENAME\\\"


INCLUDEPATH += \
    $$GCS_SOURCE_TREE/src/libs

DEPENDPATH += \
    $$GCS_SOURCE_TREE/src/libs

LIBS += -L$$GCS_LIBRARY_PATH

# DEFINES += QT_NO_CAST_FROM_ASCII
DEFINES += QT_NO_CAST_TO_ASCII
#DEFINES += QT_USE_FAST_OPERATOR_PLUS
#DEFINES += QT_USE_FAST_CONCATENATION

unix {
    CONFIG(debug, debug|release):OBJECTS_DIR = $${OUT_PWD}/.obj/debug-shared
    CONFIG(release, debug|release):OBJECTS_DIR = $${OUT_PWD}/.obj/release-shared

    CONFIG(debug, debug|release):MOC_DIR = $${OUT_PWD}/.moc/debug-shared
    CONFIG(release, debug|release):MOC_DIR = $${OUT_PWD}/.moc/release-shared

    RCC_DIR = $${OUT_PWD}/.rcc
    UI_DIR = $${OUT_PWD}/.uic
}

linux-g++-* {
    # Bail out on non-selfcontained libraries. Just a security measure
    # to prevent checking in code that does not compile on other platforms.
    QMAKE_LFLAGS += -Wl,--allow-shlib-undefined -Wl,--no-undefined
}

