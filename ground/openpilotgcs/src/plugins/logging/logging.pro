TEMPLATE = lib

TARGET = LoggingGadget
DEFINES += LOGGING_LIBRARY
QT += svg

include(../../openpilotgcsplugin.pri)
include(logging_dependencies.pri)
HEADERS += loggingplugin.h \
    logginggadgetwidget.h \
    logginggadget.h \
    logginggadgetfactory.h

SOURCES += loggingplugin.cpp \
    logginggadgetwidget.cpp \
    logginggadget.cpp \
    logginggadgetfactory.cpp

OTHER_FILES += LoggingGadget.pluginspec

FORMS += logging.ui

