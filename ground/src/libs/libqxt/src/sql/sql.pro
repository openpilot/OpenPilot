CLEAN_TARGET     = QxtSql
DEFINES         += BUILD_QXT_SQL
QT               = core sql
QXT              = core
CONVENIENCE     += $$CLEAN_TARGET

include(sql.pri)
#include(../qxtbase.pri)
