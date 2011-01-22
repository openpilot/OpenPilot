CLEAN_TARGET     = QxtZeroconf
DEFINES         += BUILD_QXT_ZEROCONF
QT               = core network
QXT              = core network
CONVENIENCE     += $$CLEAN_TARGET

include(zeroconf.pri)
#include(../qxtbase.pri)

unix:!macx:LIBS += -ldns_sd 
!contains(CONFIG,NO_AVAHI): unix:!macx:LIBS +=  -lavahi-client -lavahi-common
win32:LIBS        += -L"c:\\PROGRA~1\\BONJOU~1\\lib\\win32" -ldnssd
win32:INCLUDEPATH += "c:\\program files\\bonjour sdk\\include"
