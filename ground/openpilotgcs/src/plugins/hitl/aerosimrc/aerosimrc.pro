TEMPLATE  = subdirs

win32 {
    SUBDIRS += plugin
}

SUBDIRS += udptest

plugin.file   = src/plugin.pro
udptest.file  = src/udptest.pro
