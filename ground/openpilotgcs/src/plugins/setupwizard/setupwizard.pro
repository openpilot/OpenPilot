
TEMPLATE = lib 
TARGET = SetupWizard 
QT += svg

include(../../openpilotgcsplugin.pri) 
include(../../plugins/coreplugin/coreplugin.pri) 
include(../../plugins/uavobjectutil/uavobjectutil.pri)
include(../../plugins/config/config.pri)

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
    levellingutil.h \
    vehicleconfigurationsource.h \
    vehicleconfigurationhelper.h \
    connectiondiagram.h \
    pages/outputcalibrationpage.h \
    outputcalibrationutil.h \
    pages/rebootpage.h

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
    levellingutil.cpp \
    vehicleconfigurationsource.cpp \
    vehicleconfigurationhelper.cpp \
    connectiondiagram.cpp \
    pages/outputcalibrationpage.cpp \
    outputcalibrationutil.cpp \
    pages/rebootpage.cpp

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
    pages/levellingpage.ui \
    connectiondiagram.ui \
    pages/outputcalibrationpage.ui \
    pages/rebootpage.ui

RESOURCES += \
    wizardResources.qrc
