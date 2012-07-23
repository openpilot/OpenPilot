TEMPLATE = lib
TARGET = PfdQml
QT += svg
QT += opengl
QT += declarative

include(../../openpilotgcsplugin.pri)
include(../../plugins/coreplugin/coreplugin.pri)
include(pfdqml_dependencies.pri)

LIBS += -losg -losgUtil -losgViewer -losgQt -losgDB -lOpenThreads -losgGA
LIBS += -losgEarth -losgEarthFeatures -losgEarthUtil

HEADERS += \
    pfdqmlplugin.h \
    pfdqmlgadget.h \
    pfdqmlgadgetwidget.h \
    pfdqmlgadgetfactory.h \
    pfdqmlgadgetconfiguration.h \
    pfdqmlgadgetoptionspage.h \
    osgearth.h

SOURCES += \
    pfdqmlplugin.cpp \
    pfdqmlgadget.cpp \
    pfdqmlgadgetfactory.cpp \
    pfdqmlgadgetwidget.cpp \
    pfdqmlgadgetconfiguration.cpp \
    pfdqmlgadgetoptionspage.cpp \
    osgearth.cpp

OTHER_FILES += PfdQml.pluginspec

FORMS += pfdqmlgadgetoptionspage.ui

