TEMPLATE = lib
TARGET = OPMapGadget
include(../../openpilotgcsplugin.pri)
include(../../plugins/coreplugin/coreplugin.pri)
include(../../libs/opmapcontrol/opmapcontrol.pri)
include(../../plugins/uavobjects/uavobjects.pri)
HEADERS += opmapplugin.h \
    opmapgadgetoptionspage.h \
    opmapgadgetfactory.h \
    opmapgadgetconfiguration.h \
    opmapgadget.h \
    opmapgadgetwidget.h \
    opmap_waypointeditor_dialog.h \
    opmap_mapoverlaywidget.h
SOURCES += opmapplugin.cpp \
    opmapgadgetwidget.cpp \
    opmapgadgetoptionspage.cpp \
    opmapgadgetfactory.cpp \
    opmapgadgetconfiguration.cpp \
    opmapgadget.cpp \
    opmap_waypointeditor_dialog.cpp \
    opmap_mapoverlaywidget.cpp
OTHER_FILES += OPMapGadget.pluginspec
FORMS += opmapgadgetoptionspage.ui \
    opmap_widget.ui \
    opmap_waypointeditor_dialog.ui \
    opmap_mapoverlaywidget.ui
RESOURCES += opmap.qrc
