TEMPLATE = lib
TARGET = IPconnection
include(../../openpilotgcsplugin.pri)
include(ipconnection_dependencies.pri)
HEADERS += ipconnectionplugin.h \
    ipconnection_global.h \
    ipconnectionconfiguration.h \
    ipconnectionoptionspage.h \
    ipconnection_internal.h
SOURCES += ipconnectionplugin.cpp \
    ipconnectionconfiguration.cpp \
    ipconnectionoptionspage.cpp
FORMS += ipconnectionoptionspage.ui
RESOURCES += 
DEFINES += IPconnection_LIBRARY
OTHER_FILES += IPconnection.pluginspec
QT += network
