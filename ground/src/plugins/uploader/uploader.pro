TEMPLATE = lib
TARGET = Uploader
include(../../openpilotgcsplugin.pri)
include(../../plugins/coreplugin/coreplugin.pri)
include(../../libs/qextserialport/qextserialport.pri)
include(../../libs/qymodem/qymodem.pri)
HEADERS += uploadergadget.h \
    uploadergadgetconfiguration.h \
    uploadergadgetfactory.h \
    uploadergadgetoptionspage.h \
    uploadergadgetwidget.h \
    uploaderplugin.h
SOURCES += uploadergadget.cpp \
    uploadergadgetconfiguration.cpp \
    uploadergadgetfactory.cpp \
    uploadergadgetoptionspage.cpp \
    uploadergadgetwidget.cpp \
    uploaderplugin.cpp
OTHER_FILES += Uploader.pluginspec
