
TEMPLATE = lib 
TARGET = SetupWizard 
QT += svg

include(../../openpilotgcsplugin.pri) 
include(../../plugins/coreplugin/coreplugin.pri) 
include(../../plugins/uavobjectutil/uavobjectutil.pri)
 
HEADERS += setupwizardplugin.h \ 
    setupwizard.h \
    pages/startpage.h \
    pages/endpage.h \
    pages/controllerpage.h \
    pages/vehiclepage.h \
    pages/notyetimplementedpage.h \
    pages/multipage.h \
    pages/fixedwingpage.h \
    pages/helipage.h \
    pages/surfacepage.h \
    pages/abstractwizardpage.h \
    pages/outputpage.h \
    pages/inputpage.h \
    pages/summarypage.h \
    pages/flashpage.h \
    pages/levellingpage.h \
    levellingutil.h

SOURCES += setupwizardplugin.cpp \
    setupwizard.cpp \
    pages/startpage.cpp \
    pages/endpage.cpp \
    pages/controllerpage.cpp \
    pages/vehiclepage.cpp \
    pages/notyetimplementedpage.cpp \
    pages/multipage.cpp \
    pages/fixedwingpage.cpp \
    pages/helipage.cpp \
    pages/surfacepage.cpp \
    pages/abstractwizardpage.cpp \
    pages/outputpage.cpp \
    pages/inputpage.cpp \
    pages/summarypage.cpp \
    pages/flashpage.cpp \
    pages/levellingpage.cpp \
    levellingutil.cpp

OTHER_FILES += SetupWizard.pluginspec

FORMS += \
    pages/startpage.ui \
    pages/endpage.ui \
    pages/controllerpage.ui \
    pages/vehiclepage.ui \
    pages/notyetimplementedpage.ui \
    pages/multipage.ui \
    pages/fixedwingpage.ui \
    pages/helipage.ui \
    pages/surfacepage.ui \
    pages/outputpage.ui \
    pages/inputpage.ui \
    pages/summarypage.ui \
    pages/flashpage.ui \
    pages/levellingpage.ui

RESOURCES += \
    wizardResources.qrc
