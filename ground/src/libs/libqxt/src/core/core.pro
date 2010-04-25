TEMPLATE = lib
TARGET = QxtCore

DEFINES += QXTCORE_LIBRARY
include(../../../../openpilotgcslibrary.pri)

DEFINES += BUILD_QXT_CORE

include(core.pri)
include(logengines/logengines.pri)
#include(../qxtbase.pri)
