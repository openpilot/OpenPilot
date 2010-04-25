#ifndef QXTSERVICEBROWSER_H
#define QXTSERVICEBROWSER_H

#include "qxtglobal.h"
#include <QObject>
#include <QAbstractSocket>
#include "qxtdiscoverableservicename.h"

class QxtServiceBrowserPrivate;
class QXT_ZEROCONF_EXPORT QxtServiceBrowser : public QObject, public QxtDiscoverableServiceName
{
Q_OBJECT
QXT_DECLARE_PRIVATE(QxtServiceBrowser)
public:
    QxtServiceBrowser(const QString& serviceType, QObject* parent = 0);
    QxtServiceBrowser(const QString& serviceType, QAbstractSocket::SocketType socketType, QObject* parent = 0);
    virtual ~QxtServiceBrowser();

    bool isBrowsing() const;

public slots:
    void browse(/* int iface */);
    void stopBrowsing();

public:
    QStringList serviceSubTypes() const;
    void setServiceSubTypes(const QStringList& subtypes);
    void addServiceSubType(const QString& subtype);
    void removeServiceSubType(const QString& subtype);
    bool hasServiceSubType(const QString& subtype);

Q_SIGNALS:
    void browsingFailed(int errorCode);
    void serviceAdded(const QString& serviceName, const QString& domain);
    void serviceRemoved(const QString& serviceName, const QString& domain);
};

#endif // QXTSERVICEBROWSER_H
