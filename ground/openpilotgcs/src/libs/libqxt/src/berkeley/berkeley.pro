CLEAN_TARGET     = QxtBerkeley
DEFINES         += BUILD_QXT_BERKELEY
QT               = core
QXT              = core
CONVENIENCE     += $$CLEAN_TARGET

include(berkeley.pri)
#include(../qxtbase.pri)

!win32:LIBS     += -ldb
