TEMPLATE = lib
TARGET = OsgEarthviewGadget

QT += opengl
include(../../openpilotgcsplugin.pri)
include(../../plugins/coreplugin/coreplugin.pri)
include(osgearthview_dependencies.pri)
include(osg.pri)
include(osgearth.pri)

HEADERS += osgearthviewplugin.h \
    osgviewerwidget.h
HEADERS += osgearthviewgadget.h
HEADERS += osgearthviewwidget.h
HEADERS += osgearthviewgadgetfactory.h
HEADERS += osgearthviewgadgetconfiguration.h
HEADERS += osgearthviewgadgetoptionspage.h

SOURCES += osgearthviewplugin.cpp \
    osgviewerwidget.cpp
SOURCES += osgearthviewgadget.cpp
SOURCES += osgearthviewwidget.cpp
SOURCES += osgearthviewgadgetfactory.cpp
SOURCES += osgearthviewgadgetconfiguration.cpp
SOURCES += osgearthviewgadgetoptionspage.cpp

FORMS += osgearthviewgadgetoptionspage.ui \
    osgearthview.ui

OTHER_FILES += OsgEarthviewGadget.pluginspec
RESOURCES += osgearthview.qrc











