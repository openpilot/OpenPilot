TEMPLATE = lib
TARGET = UAVObjectWidgetUtils
DEFINES += UAVOBJECTWIDGETUTILS_LIBRARY
QT += svg
include(../../openpilotgcsplugin.pri)
include(uavobjectwidgetutils_dependencies.pri)
HEADERS += uavobjectwidgetutils_global.h \
    uavobjectwidgetutilsplugin.h \
    configtaskwidget.h \
    mixercurvewidget.h \
    mixercurvepoint.h \
    mixercurveline.h \
 smartsavebutton.h \
    mixercurve.h
SOURCES += uavobjectwidgetutilsplugin.cpp \
    configtaskwidget.cpp \
    mixercurvewidget.cpp \
    mixercurvepoint.cpp \
    mixercurveline.cpp \
    smartsavebutton.cpp \
    mixercurve.cpp


OTHER_FILES += UAVObjectWidgetUtils.pluginspec

FORMS += \
    mixercurve.ui
