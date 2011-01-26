TEMPLATE = lib
TARGET = ScopeGadget
DEFINES += SCOPE_LIBRARY
include(../../openpilotgcsplugin.pri)
include (scope_dependencies.pri)
HEADERS += scopeplugin.h \
    plotdata.h \
    scope_global.h
HEADERS += scopegadgetoptionspage.h
HEADERS += scopegadgetconfiguration.h
HEADERS += scopegadget.h
HEADERS += scopegadgetwidget.h
HEADERS += scopegadgetfactory.h
SOURCES += scopeplugin.cpp \
    plotdata.cpp
SOURCES += scopegadgetoptionspage.cpp
SOURCES += scopegadgetconfiguration.cpp
SOURCES += scopegadget.cpp
SOURCES += scopegadgetfactory.cpp
SOURCES += scopegadgetwidget.cpp
OTHER_FILES += ScopeGadget.pluginspec
FORMS += scopegadgetoptionspage.ui
