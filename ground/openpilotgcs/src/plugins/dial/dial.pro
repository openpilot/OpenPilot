TEMPLATE = lib
TARGET = DialGadget
QT += svg
QT += opengl
include(../../openpilotgcsplugin.pri)
include(../../plugins/coreplugin/coreplugin.pri)
include(dial_dependencies.pri)
HEADERS += dialplugin.h
HEADERS += dialgadget.h
HEADERS += dialgadgetwidget.h
HEADERS += dialgadgetfactory.h
HEADERS += dialgadgetconfiguration.h
HEADERS += dialgadgetoptionspage.h
SOURCES += dialplugin.cpp
SOURCES += dialgadget.cpp
SOURCES += dialgadgetfactory.cpp
SOURCES += dialgadgetwidget.cpp
SOURCES += dialgadgetconfiguration.cpp
SOURCES += dialgadgetoptionspage.cpp
OTHER_FILES += DialGadget.pluginspec
FORMS += dialgadgetoptionspage.ui
RESOURCES += dial.qrc
