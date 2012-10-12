TEMPLATE = lib
TARGET = Uploader
DEFINES += UPLOADER_LIBRARY
QT += svg
include(uploader_dependencies.pri)
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
    runningdevicewidget.h \
    uploader_global.h \
    enums.h
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
OTHER_FILES += Uploader.pluginspec \

FORMS += \
    uploader.ui \
    devicewidget.ui \
    runningdevicewidget.ui

RESOURCES += \
    uploader.qrc
exists( ../../../../../build/ground/opfw_resource/opfw_resource.qrc ) {
RESOURCES += ../../../../build/ground/opfw_resource/opfw_resource.qrc
}
