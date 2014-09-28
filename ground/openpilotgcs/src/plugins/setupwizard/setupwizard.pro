
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
    pages/opstartpage.h \
    pages/opendpage.h \
    pages/controllerpage.h \
    pages/vehiclepage.h \
    pages/notyetimplementedpage.h \
    pages/multipage.h \
    pages/fixedwingpage.h \
    pages/airspeedpage.h \
    pages/gpspage.h \
    pages/helipage.h \
    pages/surfacepage.h \
    pages/abstractwizardpage.h \
    pages/inputpage.h \
    pages/summarypage.h \
    vehicleconfigurationsource.h \
    vehicleconfigurationhelper.h \
    connectiondiagram.h \
    pages/outputcalibrationpage.h \
    outputcalibrationutil.h \
    pages/rebootpage.h \
    pages/savepage.h \
    pages/autoupdatepage.h \
    pages/revocalibrationpage.h \
    biascalibrationutil.h \
    pages/biascalibrationpage.h \
    pages/escpage.h \
    pages/servopage.h \
    pages/selectionpage.h \
    pages/airframeinitialtuningpage.h \
    vehicletemplateexportdialog.h

SOURCES += setupwizardplugin.cpp \
    setupwizard.cpp \
    pages/opstartpage.cpp \
    pages/opendpage.cpp \
    pages/controllerpage.cpp \
    pages/vehiclepage.cpp \
    pages/notyetimplementedpage.cpp \
    pages/multipage.cpp \
    pages/fixedwingpage.cpp \
    pages/airspeedpage.cpp \
    pages/gpspage.cpp \
    pages/helipage.cpp \
    pages/surfacepage.cpp \
    pages/abstractwizardpage.cpp \
    pages/inputpage.cpp \
    pages/summarypage.cpp \
    vehicleconfigurationsource.cpp \
    vehicleconfigurationhelper.cpp \
    connectiondiagram.cpp \
    pages/outputcalibrationpage.cpp \
    outputcalibrationutil.cpp \
    pages/rebootpage.cpp \
    pages/savepage.cpp \
    pages/autoupdatepage.cpp \
    pages/revocalibrationpage.cpp \
    biascalibrationutil.cpp \
    pages/biascalibrationpage.cpp \
    pages/escpage.cpp \
    pages/servopage.cpp \
    pages/selectionpage.cpp \
    pages/airframeinitialtuningpage.cpp \
    vehicletemplateexportdialog.cpp

OTHER_FILES += SetupWizard.pluginspec

FORMS += \
    pages/opstartpage.ui \
    pages/opendpage.ui \
    pages/controllerpage.ui \
    pages/vehiclepage.ui \
    pages/notyetimplementedpage.ui \
    pages/helipage.ui \
    pages/surfacepage.ui \
    pages/inputpage.ui \
    pages/summarypage.ui \
    connectiondiagram.ui \
    pages/outputcalibrationpage.ui \
    pages/rebootpage.ui \
    pages/savepage.ui \
    pages/autoupdatepage.ui \
    pages/revocalibrationpage.ui \
    pages/biascalibrationpage.ui \
    pages/escpage.ui \
    pages/servopage.ui \
    pages/selectionpage.ui \
    pages/airframeinitialtuningpage.ui \
    vehicletemplateexportdialog.ui

RESOURCES += \
    wizardResources.qrc
