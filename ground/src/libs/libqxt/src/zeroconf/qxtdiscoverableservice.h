#ifndef QXTDISCOVERABLESERVICE_H
#define QXTDISCOVERABLESERVICE_H

#include "qxtglobal.h"
#include <QObject>
#include <QFlags>
#include <QStringList>
#include <QAbstractSocket>
#include "qxtdiscoverableservicename.h"

class QxtDiscoverableServicePrivate;
class QxtServiceBrowser;
class QXT_ZEROCONF_EXPORT QxtDiscoverableService : public QObject, public QxtDiscoverableServiceName {
Q_OBJECT
QXT_DECLARE_PRIVATE(QxtDiscoverableService)
friend class QxtServiceBrowser;
private:
    QxtDiscoverableService(const QByteArray& domainName, QxtServiceBrowser* parent);

public:
    enum State {
        Unknown,
        Registering,
        Registered,
        Found,
        Resolving,
        Resolved
    };

    enum ServiceFlag {
        MoreComing = 0x1,

        Add = 0x2,
        Remove = 0x0,
        AddDefault = 0x6,
        ForceRemove = 0x800,

        AutoRename = 0x0,
        NoAutoRename = 0x8,

        RecordShared = 0x10,
        RecordUnique = 0x20,

        BrowseDomains = 0x40,
        RegistrationDomains = 0x80,

        LongLivedQuery = 0x100,
        AllowRemoteQuery = 0x200,
        ForceMulticast = 0x400,

        ReturnIntermediates = 0x1000,
        NonBrowsable = 0x2000,
        ShareConnection = 0x4000
    };
    Q_DECLARE_FLAGS(ServiceFlags, ServiceFlag);

    enum ErrorCode {
        NoError = 0
    };

    QxtDiscoverableService(const QString& serviceType, QObject* parent = 0);
    QxtDiscoverableService(const QString& serviceType, const QString& serviceName, QObject* parent = 0);
    virtual ~QxtDiscoverableService();

    QStringList serviceSubTypes() const;
    void setServiceSubTypes(const QStringList& subtypes);
    void addServiceSubType(const QString& subtype);
    void removeServiceSubType(const QString& subtype);
    bool hasServiceSubType(const QString& subtype);

    quint16 port() const;
    void setPort(quint16 port);

    void resolve(bool forceMulticast = false);
    void registerService(bool noAutoRename = false);
    void releaseService();
    State state() const;

    //bool addRecord(RecordType type, const QByteArray& record, int ttl = 0);


Q_SIGNALS:
    void registered();
    void registrationError(int code);

    void resolved(const QByteArray& domainName);
    void resolveError(int code);
};
Q_DECLARE_OPERATORS_FOR_FLAGS(QxtDiscoverableService::ServiceFlags);

#endif // QXTDISCOVERABLESERVICE_H
