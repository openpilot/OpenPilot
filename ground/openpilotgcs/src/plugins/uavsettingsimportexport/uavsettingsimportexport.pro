
TEMPLATE = lib
QT += xml

TARGET = UAVSettingsImportExport
 
include(../../openpilotgcsplugin.pri)
include(uavsettingsimportexport_dependencies.pri)

HEADERS += uavsettingsimportexport.h \
    importsummary.h
SOURCES += uavsettingsimportexport.cpp \
    importsummary.cpp
 
OTHER_FILES += uavsettingsimportexport.pluginspec

FORMS += \
    importsummarydialog.ui
