CLEAN_TARGET     = QxtWeb
DEFINES         += BUILD_QXT_WEB
QT               = core network
QXT              = core network
CONVENIENCE     += $$CLEAN_TARGET

include(web.pri)
#include(../qxtbase.pri)
