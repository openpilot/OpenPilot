TEMPLATE = lib
TARGET = ScopeGadget

DEFINES += SCOPE_LIBRARY

include(../../openpilotgcsplugin.pri)
include (scope_dependencies.pri)

HEADERS += \
    scopeplugin.h \
    plotdata.h \
    scope_global.h \
    scopegadgetoptionspage.h \
    scopegadgetconfiguration.h \
    scopegadget.h \
    scopegadgetwidget.h \
    scopegadgetfactory.h

SOURCES += \
    scopeplugin.cpp \
    plotdata.cpp \
    scopegadgetoptionspage.cpp \
    scopegadgetconfiguration.cpp \
    scopegadget.cpp \
    scopegadgetfactory.cpp \
    scopegadgetwidget.cpp

OTHER_FILES += ScopeGadget.pluginspec

FORMS += scopegadgetoptionspage.ui
