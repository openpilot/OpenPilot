INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += qxtdiscoverableservice.h
HEADERS += qxtdiscoverableservice_p.h
HEADERS += qxtservicebrowser.h
HEADERS += qxtservicebrowser_p.h
HEADERS += qxtdiscoverableservicename.h

SOURCES += qxtdiscoverableservice.cpp
SOURCES += qxtservicebrowser.cpp
SOURCES += qxtdiscoverableservicename.cpp

!contains(CONFIG,NO_AVAHI): unix : !macx {
        DEFINES += USE_AVAHI
        SOURCES += qxtmdns_avahi.cpp
        SOURCES += qxtavahipoll.cpp
        HEADERS += qxtmdns_avahi.h
        HEADERS += qxtmdns_avahi_p.h
        HEADERS += qxtavahipoll.h
        HEADERS += qxtavahipoll_p.h
} else {
        SOURCES += qxtmdns_bonjour.cpp
        HEADERS += qxtmdns_bonjour.h
}
