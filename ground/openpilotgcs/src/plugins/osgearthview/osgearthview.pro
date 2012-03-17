TEMPLATE = lib
TARGET = PFDGadget

QT += opengl
include(../../openpilotgcsplugin.pri)
include(../../plugins/coreplugin/coreplugin.pri)
include(pfd_dependencies.pri)
include(osg.pri)
include(osgearth.pri)

HEADERS += osgearthviewplugin.h
HEADERS += osgearthviewgadget.h
HEADERS += osgearthviewwidget.h
HEADERS += osgearthviewgadgetfactory.h
HEADERS += osgearthviewgadgetconfiguration.h
HEADERS += osgearthviewgadgetoptionspage.h

SOURCES += osgearthviewplugin.cpp
SOURCES += osgearthviewgadget.cpp
SOURCES += osgearthviewwidget.cpp
SOURCES += osgearthviewgadgetfactory.cpp
SOURCES += osgearthviewgadgetconfiguration.cpp
SOURCES += osgearthviewgadgetoptionspage.cpp

OTHER_FILES += OsgEarthview.pluginspec
RESOURCES += osgearthview.qrc
