CLEAN_TARGET     = QxtNetwork
DEFINES         += BUILD_QXT_NETWORK
QT               = core network
QXT              = core
CONVENIENCE     += $$CLEAN_TARGET

include(network.pri)
#include(../qxtbase.pri)
