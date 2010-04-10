TEMPLATE = lib
TARGET = Uploader
include(../../openpilotgcsplugin.pri)
include(../../plugins/coreplugin/coreplugin.pri)
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
OTHER_FILES += uploader.pluginspec

LIBS += -l$$qtLibraryTarget(QExtSerialPort)
LIBS += -l$$qtLibraryTarget(QYmodem)

#CONFIG(debug, debug|release):LIBS += -lqextserialportd
#else:LIBS += -lqextserialport
#CONFIG(debug, debug|release):LIBS += -lqymodemd
#else:LIBS += -lqymodem
