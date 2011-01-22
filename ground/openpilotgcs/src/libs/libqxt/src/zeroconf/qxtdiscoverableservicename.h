#ifndef QXTDISCOVERABLESERVICENAME_H
#define QXTDISCOVERABLESERVICENAME_H

#include "qxtglobal.h"
#include <QString>
#include <QByteArray>
#include <QAbstractSocket>

class QxtDiscoverableServiceNamePrivate;
class QXT_ZEROCONF_EXPORT QxtDiscoverableServiceName
{
QXT_DECLARE_PRIVATE(QxtDiscoverableServiceName)
public:
    QxtDiscoverableServiceName();
    QxtDiscoverableServiceName(const QByteArray& domainName);
    QxtDiscoverableServiceName(const QString& name, const QString& serviceType, const QString& domain,
                               QAbstractSocket::SocketType socketType = QAbstractSocket::UnknownSocketType);

    QString serviceName() const;
    void setServiceName(const QString& name);

    QAbstractSocket::SocketType socketType() const;
    void setSocketType(QAbstractSocket::SocketType type);

    QString serviceType() const;
    void setServiceType(const QString& type);
    QByteArray fullServiceType() const;

    QString domain() const;
    void setDomain(const QString& domain);

    QString host() const;
    void setHost(const QString& host);

    QByteArray escapedFullDomainName() const;

    //mDNS stuff
    static int lookupHost(const QString name, QObject* receiver, const char* member);
    static void abortHostLookup(int id);
};

#endif // QXTDISCOVERABLESERVICENAME_H
