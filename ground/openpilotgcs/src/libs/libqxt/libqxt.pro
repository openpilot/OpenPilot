#
# Qxt main project file
#
#
TEMPLATE = subdirs

QXT_MODULES = core

contains( QXT_MODULES, core ){
    message( building core module )
    sub_core.subdir = src/core
    SUBDIRS += sub_core
}

contains( QXT_MODULES, gui ){
    message( building gui module )
    sub_gui.subdir = src/gui
    sub_gui.depends = sub_core
    SUBDIRS += sub_gui
    contains( QXT_MODULES, designer ){
        sub_designer.subdir = src/designer
        sub_designer.depends = sub_core sub_gui
        SUBDIRS += sub_designer
    }
}

contains( QXT_MODULES, network ){
    message( building network module )
    sub_network.subdir = src/network
    sub_network.depends = sub_core
    SUBDIRS += sub_network
}

contains( QXT_MODULES, sql ){
    message( building sql module )
    sub_sql.subdir = src/sql
    sub_sql.depends = sub_core
    SUBDIRS += sub_sql
}

contains(DEFINES,HAVE_DB){
contains( QXT_MODULES, berkeley ){
    message( building berkeley module )
    sub_berkeley.subdir = src/berkeley
    sub_berkeley.depends = sub_core
    SUBDIRS += sub_berkeley
}
}

contains(DEFINES,HAVE_ZEROCONF){
contains( QXT_MODULES, zeroconf ){
    message( building zeroconf module )
    sub_zeroconf.subdir = src/zeroconf
    sub_zeroconf.depends = sub_network
    SUBDIRS += sub_zeroconf
}
}

contains( QXT_MODULES, web ){
    message( building web module )
    sub_web.subdir = src/web
    sub_web.depends = sub_core sub_network
    SUBDIRS += sub_web
}


