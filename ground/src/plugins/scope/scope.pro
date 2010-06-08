TEMPLATE = lib
TARGET = ScopeGadget
include(../../openpilotgcsplugin.pri)
include(../../plugins/coreplugin/coreplugin.pri)
include(../../plugins/uavobjects/uavobjects.pri)
include(../../libs/qwt/qwt.pri)
HEADERS += scopeplugin.h \
    plotdata.h
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
