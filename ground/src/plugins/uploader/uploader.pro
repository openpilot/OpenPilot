TEMPLATE = lib
TARGET = Uploader
include(../../openpilotgcsplugin.pri)
include(../../plugins/coreplugin/coreplugin.pri)
include(../../plugins/uavobjects/uavobjects.pri)
include(../../libs/qextserialport/qextserialport.pri)
include(../../plugins/rawhid/rawhid.pri)
include(../../libs/qymodem/qymodem.pri)
HEADERS += uploadergadget.h \
    uploadergadgetconfiguration.h \
    uploadergadgetfactory.h \
    uploadergadgetoptionspage.h \
    uploadergadgetwidget.h \
    uploaderplugin.h \
    op_dfu.h \
    delay.h
SOURCES += uploadergadget.cpp \
    uploadergadgetconfiguration.cpp \
    uploadergadgetfactory.cpp \
    uploadergadgetoptionspage.cpp \
    uploadergadgetwidget.cpp \
    uploaderplugin.cpp \
    op_dfu.cpp \
    delay.cpp
OTHER_FILES += Uploader.pluginspec

FORMS += \
    uploader.ui
