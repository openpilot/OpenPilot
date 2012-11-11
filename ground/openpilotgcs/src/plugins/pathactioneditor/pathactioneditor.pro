TEMPLATE = lib
TARGET = PathActionEditor 

include(../../openpilotgcsplugin.pri) 
include(../../plugins/coreplugin/coreplugin.pri) 
include(../../plugins/uavobjects/uavobjects.pri)

HEADERS += pathactioneditorgadget.h
HEADERS += pathactioneditorgadgetwidget.h
HEADERS += pathactioneditorgadgetfactory.h
HEADERS += pathactioneditorplugin.h
HEADERS += pathactioneditortreemodel.h
HEADERS += treeitem.h
HEADERS += fieldtreeitem.h
HEADERS += browseritemdelegate.h

SOURCES += pathactioneditorgadget.cpp
SOURCES += pathactioneditorgadgetwidget.cpp
SOURCES += pathactioneditorgadgetfactory.cpp
SOURCES += pathactioneditorplugin.cpp
SOURCES += pathactioneditortreemodel.cpp
SOURCES += treeitem.cpp
SOURCES += fieldtreeitem.cpp
SOURCES += browseritemdelegate.cpp


OTHER_FILES += pathactioneditor.pluginspec

FORMS += pathactioneditor.ui

RESOURCES += pathactioneditor.qrc


