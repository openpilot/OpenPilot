
TEMPLATE = lib 
TARGET = SetupWizard 
 
include(../../openpilotgcsplugin.pri) 
include(../../plugins/coreplugin/coreplugin.pri) 
 
HEADERS += setupwizardplugin.h \ 
    setupwizard.h \
    pages/startpage.h \
    pages/endpage.h
SOURCES += setupwizardplugin.cpp \ 
    setupwizard.cpp \
    pages/startpage.cpp \
    pages/endpage.cpp
 
OTHER_FILES += SetupWizard.pluginspec

FORMS += \
    pages/startpage.ui \
    pages/endpage.ui