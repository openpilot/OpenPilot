TEMPLATE = lib
TARGET = Uploader
QT += svg
include(../../openpilotgcsplugin.pri)
include(../../plugins/coreplugin/coreplugin.pri)
include(../../plugins/uavobjects/uavobjects.pri)
include(../../plugins/uavtalk/uavtalk.pri)
include(../../plugins/rawhid/rawhid.pri)
include(../../plugins/uavobjectutil/uavobjectutil.pri)
INCLUDEPATH += ../../libs/qextserialport/src

HEADERS += uploadergadget.h \
    uploadergadgetconfiguration.h \
    uploadergadgetfactory.h \
    uploadergadgetoptionspage.h \
    uploadergadgetwidget.h \
    uploaderplugin.h \
    op_dfu.h \
    delay.h \
    devicewidget.h \
    SSP/port.h \
    SSP/qssp.h \
    SSP/qsspt.h \
    SSP/common.h \
    runningdevicewidget.h
SOURCES += uploadergadget.cpp \
    uploadergadgetconfiguration.cpp \
    uploadergadgetfactory.cpp \
    uploadergadgetoptionspage.cpp \
    uploadergadgetwidget.cpp \
    uploaderplugin.cpp \
    op_dfu.cpp \
    delay.cpp \
    devicewidget.cpp \
    SSP/port.cpp \
    SSP/qssp.cpp \
    SSP/qsspt.cpp \
    runningdevicewidget.cpp
OTHER_FILES += Uploader.pluginspec

FORMS += \
    uploader.ui \
    devicewidget.ui \
    runningdevicewidget.ui

RESOURCES += \
    uploader.qrc
exists( ../../../../build/ground/opfw_resource/opfw_resource.qrc ) {
     \ ../../../../build/ground/opfw_resource/opfw_resource.qrc
}
