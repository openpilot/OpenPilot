#ifndef QXTSERVICEBROWSER_P_H
#define QXTSERVICEBROWSER_P_H

#include "qxtservicebrowser.h"
#include <dns_sd.h>
#include <QStringList>

class QSocketNotifier;
class QxtServiceBrowserPrivate : public QObject, public QxtPrivate<QxtServiceBrowser>
{
Q_OBJECT
public:
    QXT_DECLARE_PUBLIC(QxtServiceBrowser)
    QxtServiceBrowserPrivate() {
        notifier = 0;
        iface = 0;
    }

    DNSServiceRef service;

    QString domain;
    QAbstractSocket::SocketType socketType;
    QString serviceType;
    QStringList serviceSubTypes;
    int iface;

    QSocketNotifier* notifier;

    static void DNSSD_API browseServiceCallback(DNSServiceRef service, DNSServiceFlags flags, quint32 iface, DNSServiceErrorType errCode,
                                                const char* serviceName, const char* regtype, const char* replyDomain, void* context);

public Q_SLOTS:
    void socketData();
};

#endif // QXTSERVICEBROWSER_P_H
