#include(config_dependencies.pri)
include(../../plugins/uavsettingsimportexport/uavsettingsimportexport.pri)
include(../../plugins/uavtalk/uavtalk.pri)
# Add the include path to the built-in uavobject include files.
INCLUDEPATH += $$PWD

LIBS *= -l$$qtLibraryName(Config)
