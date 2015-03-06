TEMPLATE = lib 
TARGET = FileSync

QT += qml quick

include(../../openpilotgcsplugin.pri)
include(../../plugins/coreplugin/coreplugin.pri)
include(../../plugins/uavobjects/uavobjects.pri)
include(../../plugins/uavobjectutil/uavobjectutil.pri)
include(../../plugins/uavtalk/uavtalk.pri)

HEADERS += filesyncplugin.h \
    filesyncmanager.h
SOURCES += filesyncplugin.cpp \
    filesyncmanager.cpp

OTHER_FILES += FileSync.pluginspec \
    FileSyncDialog.qml \
    functions.js

FORMS +=

RESOURCES += \
    fileSync.qrc
