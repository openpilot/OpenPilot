
TEMPLATE = lib 
TARGET = SetupWizard 
QT += svg


include(../../openpilotgcsplugin.pri)
include(../../plugins/coreplugin/coreplugin.pri)
include(../../plugins/uavobjectutil/uavobjectutil.pri)
include(../../plugins/config/config.pri)

LIBS *= -l$$qtLibraryName(Uploader)
HEADERS += setupwizardplugin.h \ 
    setupwizard.h \
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
    pages/levellingpage.h \
    levellingutil.h \
    vehicleconfigurationsource.h \
    vehicleconfigurationhelper.h \
    connectiondiagram.h \
    pages/outputcalibrationpage.h \
    outputcalibrationutil.h \
    pages/rebootpage.h \
    pages/savepage.h \
    pages/autoupdatepage.h \
    pages/opendpage.h \
    pages/opstartpage.h

SOURCES += setupwizardplugin.cpp \
    setupwizard.cpp \
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
    pages/levellingpage.cpp \
    levellingutil.cpp \
    vehicleconfigurationsource.cpp \
    vehicleconfigurationhelper.cpp \
    connectiondiagram.cpp \
    pages/outputcalibrationpage.cpp \
    outputcalibrationutil.cpp \
    pages/rebootpage.cpp \
    pages/savepage.cpp \
    pages/autoupdatepage.cpp \
    pages/opstartpage.cpp \
    pages/opendpage.cpp

OTHER_FILES += SetupWizard.pluginspec

FORMS += \
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
    pages/levellingpage.ui \
    connectiondiagram.ui \
    pages/outputcalibrationpage.ui \
    pages/rebootpage.ui \
    pages/savepage.ui \
    pages/autoupdatepage.ui \
    pages/opendpage.ui \
    pages/opstartpage.ui

RESOURCES += \
    wizardResources.qrc
