TEMPLATE = lib
TARGET = ImportExportGadget
DEFINES += IMPORTEXPORT_LIBRARY
QT += xml
include(../../openpilotgcsplugin.pri)
include(importexport_dependencies.pri)
HEADERS += importexportplugin.h \
    importexportgadgetwidget.h \
    importexportdialog.h
SOURCES += importexportplugin.cpp \
    importexportgadgetwidget.cpp \
    importexportdialog.cpp
OTHER_FILES += ImportExportGadget.pluginspec
FORMS += importexportgadgetwidget.ui \
    importexportdialog.ui
