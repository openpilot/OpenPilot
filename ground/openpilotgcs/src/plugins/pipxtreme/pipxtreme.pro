TEMPLATE = lib
TARGET = PipXtreme

QT += svg
QT += opengl

include(../../openpilotgcsplugin.pri)
include(../../plugins/coreplugin/coreplugin.pri)
include(../../plugins/uavobjects/uavobjects.pri)
include(../../plugins/uavtalk/uavtalk.pri)
include(../../plugins/rawhid/rawhid.pri)

INCLUDEPATH += ../../libs/qextserialport/src

HEADERS += pipxtremegadget.h \
    pipxtremegadgetconfiguration.h \
    pipxtremegadgetfactory.h \
    pipxtremegadgetoptionspage.h \
    pipxtremegadgetwidget.h \
    pipxtremeplugin.h \

SOURCES += pipxtremegadget.cpp \
    pipxtremegadgetconfiguration.cpp \
    pipxtremegadgetfactory.cpp \
    pipxtremegadgetoptionspage.cpp \
    pipxtremegadgetwidget.cpp \
    pipxtremeplugin.cpp \

OTHER_FILES += pipxtreme.pluginspec

FORMS += \
    pipxtreme.ui \
    pipxtremegadgetoptionspage.ui

RESOURCES += \
    pipxtreme.qrc
