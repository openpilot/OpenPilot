
TEMPLATE = lib
QT += xml

TARGET = UAVSettingsImportExport
 
include(../../openpilotgcsplugin.pri)
include(uavsettingsimportexport_dependencies.pri)
 
HEADERS += uavsettingsimportexport.h
SOURCES += uavsettingsimportexport.cpp
 
OTHER_FILES += uavsettingsimportexport.pluginspec
