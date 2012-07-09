
TEMPLATE = lib 
TARGET = SetupWizard 
 
include(../../openpilotgcsplugin.pri) 
include(../../plugins/coreplugin/coreplugin.pri) 
 
HEADERS += setupwizardplugin.h \ 
    setupwizard.h
SOURCES += setupwizardplugin.cpp \ 
    setupwizard.cpp
 
OTHER_FILES += SetupWizard.pluginspec