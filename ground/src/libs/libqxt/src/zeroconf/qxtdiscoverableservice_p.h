#ifndef QXTDISCOVERABLESERVICE_P_H
#define QXTDISCOVERABLESERVICE_P_H

#include "qxtdiscoverableservice.h"
#include <dns_sd.h>

class QSocketNotifier;
class QxtDiscoverableServicePrivate : public QObject, public QxtPrivate<QxtDiscoverableService>
{
Q_OBJECT
public:
    QXT_DECLARE_PUBLIC(QxtDiscoverableService)
    QxtDiscoverableServicePrivate()
    {
        port = 0;
        iface = 0;
        notifier = 0;
        state = QxtDiscoverableService::Unknown;
    }

    QxtDiscoverableService::State state;

    DNSServiceRef service;
    QStringList serviceSubTypes;
    quint16 port;
    int iface;
    QString txtRecord;

    QSocketNotifier* notifier;

    static void DNSSD_API registerServiceCallback(DNSServiceRef service, DNSServiceFlags flags, DNSServiceErrorType errCode,
                                                  const char* name, const char* regtype, const char* domain, void* context);
#ifdef Q_OS_WIN
    static void DNSSD_API resolveServiceCallback(DNSServiceRef service, DNSServiceFlags flags, quint32 iface,
                                                 DNSServiceErrorType errCode, const char* fullname, const char* host, quint16 port,
                                                 quint16 txtLen, const char* txt, void* context);
#else
    static void resolveServiceCallback(DNSServiceRef service, DNSServiceFlags flags, quint32 iface,
                                       DNSServiceErrorType errCode, const char* fullname, const char* host, quint16 port,
                                       quint16 txtLen, const unsigned char* txt, void* context);
#endif

public Q_SLOTS:
    void socketData();
};

template <typename T>
void qxt_zeroconf_parse_subtypes(T* t, const QByteArray& regtype) {
    QList<QByteArray> subtypes = regtype.split(',');
    QList<QByteArray> rt = subtypes[0].split('.');
    int ct = subtypes.count();
    t->serviceSubTypes.clear();
    if(ct > 1) {
        for(int i = 1; i < ct; i++) {
            if(subtypes.at(i)[0] == '_') {
                t->serviceSubTypes.append(subtypes[i].mid(1));
            } else {
                t->serviceSubTypes.append(subtypes[i]);
            }
        }
    }
}

#endif // QXTDISCOVERABLESERVICE_P_H
