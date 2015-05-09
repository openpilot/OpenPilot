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

defineTest(addCopyFileTarget) {
    file = $$1
    src  = $$2/$$1
    dest = $$3/$$1

    $${file}.target    = $$dest
    $${file}.depends   = $$src

    # create directory. Better would be an order only dependency
    $${file}.commands  = -@$(MKDIR) \"$$dirname(dest)\" $$addNewline()
    $${file}.commands += $(COPY_FILE) \"$$src\" \"$$dest\"

    QMAKE_EXTRA_TARGETS += $$file
    POST_TARGETDEPS += $$eval($${file}.target)

    export($${file}.target)
    export($${file}.depends)
    export($${file}.commands)
    export(QMAKE_EXTRA_TARGETS)
    export(POST_TARGETDEPS)

    return(true)
}

defineTest(addCopyDirTarget) {
    dir  = $$1
    src  = $$2/$$1
    dest = $$3/$$1

    $${dir}.target    = $$dest
    $${dir}.depends   = $$src
    # Windows does not update directory timestamp if files are modified
    win32: $${dir}.depends += FORCE

    $${dir}.commands  = @rm -rf \"$$dest\" $$addNewline()
    # create directory. Better would be an order only dependency
    $${dir}.commands += -@$(MKDIR) \"$$dirname(dest)\" $$addNewline()
    $${dir}.commands += $(COPY_DIR) \"$$src\" \"$$dest\"

    QMAKE_EXTRA_TARGETS += $$dir
    POST_TARGETDEPS += $$eval($${dir}.target)

    export($${dir}.target)
    export($${dir}.depends)
    export($${dir}.commands)
    export(QMAKE_EXTRA_TARGETS)
    export(POST_TARGETDEPS)

    return(true)
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
    out ~= s|^$$re_escape($$_PRO_FILE_PWD_/)||$$i_flag
    return($$out)
}

isEmpty(TEST):CONFIG(debug, debug|release) {
    !debug_and_release|build_pass {
        TEST = 1
    }
}

equals(TEST, 1) {
    QT +=testlib
    DEFINES += WITH_TESTS
}

#ideally, we would want a qmake.conf patch, but this does the trick...
win32:!isEmpty(QMAKE_SH):QMAKE_COPY_DIR = cp -r -f

GCS_SOURCE_TREE = $$PWD
ROOT_DIR = $$GCS_SOURCE_TREE/../..

isEmpty(GCS_BUILD_TREE) {
    sub_dir = $$_PRO_FILE_PWD_
    sub_dir ~= s,^$$re_escape($$PWD),,
    GCS_BUILD_TREE = $$cleanPath($$OUT_PWD)
    GCS_BUILD_TREE ~= s,$$re_escape($$sub_dir)$,,
}

# Find the tools directory,
# try from Makefile (not run by Qt Creator),
TOOLS_DIR = $$(TOOLS_DIR)
isEmpty(TOOLS_DIR) {
    # check for custom enviroment variable,
    TOOLS_DIR = $$(OPENPILOT_TOOLS_DIR)
    # fallback to default location.
    isEmpty(TOOLS_DIR):TOOLS_DIR = $$clean_path($$ROOT_DIR/tools)
}

GCS_APP_TARGET = openpilotgcs
macx {
    GCS_PATH = $$GCS_BUILD_TREE/$${GCS_APP_TARGET}.app/Contents
    GCS_APP_PATH = $$GCS_PATH/MacOS
    GCS_LIBRARY_PATH = $$GCS_PATH/Plugins
    GCS_PLUGIN_PATH  = $$GCS_LIBRARY_PATH
    GCS_QT_QML_PATH = $$GCS_PATH/Imports
    GCS_DATA_PATH    = $$GCS_PATH/Resources
    GCS_DOC_PATH     = $$GCS_DATA_PATH/doc
    copydata = 1
    copyqt = 1
} else {
    GCS_PATH         = $$GCS_BUILD_TREE
    GCS_APP_PATH     = $$GCS_PATH/bin
    GCS_LIBRARY_PATH = $$GCS_PATH/lib/openpilotgcs
    GCS_PLUGIN_PATH  = $$GCS_LIBRARY_PATH/plugins
    GCS_DATA_PATH    = $$GCS_PATH/share/openpilotgcs
    GCS_DOC_PATH     = $$GCS_PATH/share/doc

    !isEqual(GCS_SOURCE_TREE, $$GCS_BUILD_TREE):copydata = 1

    win32 {
        SDL_DIR = $$(SDL_DIR)
        isEmpty(SDL_DIR):SDL_DIR = $${TOOLS_DIR}/SDL-1.2.15

        OPENSSL_DIR = $$(OPENSSL_DIR)
        isEmpty(OPENSSL_DIR):OPENSSL_DIR = $${TOOLS_DIR}/openssl-1.0.1e-win32

        MESAWIN_DIR = $$(MESAWIN_DIR)
        isEmpty(MESAWIN_DIR):MESAWIN_DIR = $${TOOLS_DIR}/mesawin

        GCS_QT_PLUGINS_PATH = $$GCS_APP_PATH
        GCS_QT_QML_PATH = $$GCS_APP_PATH

        copyqt = $$copydata
    } else {
        GCS_QT_BASEPATH = $$GCS_LIBRARY_PATH/qt5
        GCS_QT_LIBRARY_PATH = $$GCS_QT_BASEPATH/lib
        GCS_QT_PLUGINS_PATH = $$GCS_QT_BASEPATH/plugins
        GCS_QT_QML_PATH = $$GCS_QT_BASEPATH/qml

        QT_INSTALL_DIR = $$clean_path($$[QT_INSTALL_LIBS]/../../../..)
        equals(QT_INSTALL_DIR, $$TOOLS_DIR) {
            copyqt = 1
        } else {
            copyqt = 0
        }
    }
}


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

win32 {
    # Fix ((packed)) pragma handling issue introduced when upgrading MinGW from 4.4 to 4.8
    # See http://gcc.gnu.org/bugzilla/show_bug.cgi?id=52991
    # The ((packet)) pragma is used in uav metadata struct and other places
    QMAKE_CXXFLAGS += -mno-ms-bitfields
}
