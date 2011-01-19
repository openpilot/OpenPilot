TEMPLATE = lib
TARGET = Serial
include(../../openpilotgcsplugin.pri)
include(serial_dependencies.pri)
INCLUDEPATH += ../../libs/qextserialport/src
HEADERS += serialplugin.h
#HEADERS += serialplugin.h \
#    serial_global.h
SOURCES += serialplugin.cpp
FORMS += 
RESOURCES += 
#DEFINES += SERIAL_LIBRARY
OTHER_FILES += Serial.pluginspec
