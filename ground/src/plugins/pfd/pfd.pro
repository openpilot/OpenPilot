TEMPLATE = lib
TARGET = PFDGadget
QT += svg
QT += opengl
include(../../openpilotgcsplugin.pri)
include(../../plugins/coreplugin/coreplugin.pri)
include(pfd_dependencies.pri)
HEADERS += pfdplugin.h
HEADERS += pfdgadget.h
HEADERS += pfdgadgetwidget.h
HEADERS += pfdgadgetfactory.h
HEADERS += pfdgadgetconfiguration.h
HEADERS += pfdgadgetoptionspage.h
SOURCES += pfdplugin.cpp
SOURCES += pfdgadget.cpp
SOURCES += pfdgadgetfactory.cpp
SOURCES += pfdgadgetwidget.cpp
SOURCES += pfdgadgetconfiguration.cpp
SOURCES += pfdgadgetoptionspage.cpp
OTHER_FILES += PFDGadget.pluginspec
FORMS += pfdgadgetoptionspage.ui
RESOURCES += pfd.qrc
