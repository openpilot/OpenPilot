TEMPLATE = lib
TARGET = LoggingGadget
DEFINES += LOGGING_LIBRARY
QT += svg
include(../../openpilotgcsplugin.pri)
include(../../plugins/uavobjects/uavobjects.pri)
include(logging_dependencies.pri)
HEADERS += loggingplugin.h
#    logginggadgetwidget.h \
#    loggingdialog.h \
#    logginggadget.h \
#    logginggadgetfactory.h \
#    logginggadgetconfiguration.h
#   logginggadgetoptionspage.h

SOURCES += loggingplugin.cpp
#    logginggadgetwidget.cpp \
#    loggingdialog.cpp \
#    logginggadget.cpp \
#    logginggadgetfactory.cpp \
#    logginggadgetconfiguration.cpp \
#    logginggadgetoptionspage.cpp
OTHER_FILES += LoggingGadget.pluginspec
#FORMS += logginggadgetoptionspage.ui \
#    logginggadgetwidget.ui \
#    loggingdialog.ui
