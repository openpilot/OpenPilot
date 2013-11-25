TEMPLATE = lib 
TARGET = FlightLog

QT += qml quick

include(../../openpilotgcsplugin.pri)
include(../../plugins/coreplugin/coreplugin.pri)
include(../../plugins/uavobjects/uavobjects.pri)

HEADERS += flightlogplugin.h \
    flightlogmanager.h \
    flightlogdialog.h
SOURCES += flightlogplugin.cpp \
    flightlogmanager.cpp \
    flightlogdialog.cpp

OTHER_FILES += Flightlog.pluginspec \
    FlightLogDialog.qml \
    functions.js

FORMS +=

RESOURCES += \
    flightLog.qrc
