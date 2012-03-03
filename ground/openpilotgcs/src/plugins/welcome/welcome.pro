TEMPLATE = lib
TARGET = Welcome
QT += network declarative

include(../../openpilotgcsplugin.pri)
include(welcome_dependencies.pri)

HEADERS += welcomeplugin.h \
    welcomemode.h \
    welcome_global.h
SOURCES += welcomeplugin.cpp \
    welcomemode.cpp \

RESOURCES += welcome.qrc
DEFINES += WELCOME_LIBRARY
OTHER_FILES += Welcome.pluginspec
