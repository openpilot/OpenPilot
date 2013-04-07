TEMPLATE = lib
TARGET = Serial
include(../../openpilotgcsplugin.pri)
include(serial_dependencies.pri)
INCLUDEPATH += ../../libs/qextserialport/src
HEADERS += serialplugin.h \
            serialpluginconfiguration.h \
            serialpluginoptionspage.h
SOURCES += serialplugin.cpp \
            serialpluginconfiguration.cpp \
            serialpluginoptionspage.cpp
FORMS += \ 
    serialpluginoptions.ui
RESOURCES += 
OTHER_FILES += Serial.pluginspec
