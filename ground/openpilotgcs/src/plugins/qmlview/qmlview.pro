TEMPLATE = lib
TARGET = QMLView
QT += svg
QT += opengl
QT += declarative

include(../../openpilotgcsplugin.pri)
include(../../plugins/coreplugin/coreplugin.pri)
include(qmlview_dependencies.pri)

HEADERS += \
    qmlviewplugin.h \
    qmlviewgadget.h \
    qmlviewgadgetwidget.h \
    qmlviewgadgetfactory.h \
    qmlviewgadgetconfiguration.h \
    qmlviewgadgetoptionspage.h


SOURCES += \
    qmlviewplugin.cpp \
    qmlviewgadget.cpp \
    qmlviewgadgetfactory.cpp \
    qmlviewgadgetwidget.cpp \
    qmlviewgadgetconfiguration.cpp \
    qmlviewgadgetoptionspage.cpp

OTHER_FILES += QMLView.pluginspec

FORMS += qmlviewgadgetoptionspage.ui

