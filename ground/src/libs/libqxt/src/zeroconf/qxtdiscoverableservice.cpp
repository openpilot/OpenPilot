#include "qxtdiscoverableservice.h"
#include "qxtdiscoverableservice_p.h"
#include <QSocketNotifier>
#include <QList>
#include <QPair>
#include <QtDebug>
#include <QtEndian>

/*!
\class QxtDiscoverableService

\inmodule QxtZeroconf

\brief The QxtDiscoverableService class registers a service that can be discovered with Zeroconf, or resolve such a service

QxtDiscoverableService represents a service on the local network that can be discovered using Zeroconf.
It can function to provide such a service so that other systems on the network can find it, or it can
resolve the connection parameters for a service provided by another system on the network. Note that
resolving a service requires that you already know the service name in addition to the service type for
the remote service; to find all services on the network that offer a given service type, use
QxtServiceBrowser.

When registering a service, you may specify a set of service subtypes to indicate what kind of optional
capabilities your service offers. Other applications browsing for discoverable services (for instance,
using QxtServiceBrowser) can limit their search to only those services that provide a given subtype.
Use the properties inherited from QxtDiscoverableServiceName to configure the other service parameters.

For more information about Zeroconf, see the documentation available on www.zeroconf.org.

\sa QxtDiscoverableServiceName
\sa QxtServiceBrowser
*/

void QxtDiscoverableServicePrivate::registerServiceCallback(DNSServiceRef service, DNSServiceFlags flags,
        DNSServiceErrorType errCode, const char* name, const char* regtype, const char* domain, void* context)
{
    Q_UNUSED(service);
    Q_UNUSED(flags);
    Q_UNUSED(regtype);
    QxtDiscoverableServicePrivate* self = reinterpret_cast<QxtDiscoverableServicePrivate*>(context);
    QxtDiscoverableService* pub = &(self->qxt_p());
    if(errCode == kDNSServiceErr_NoError) {
        pub->setServiceName(name);
        pub->setDomain(domain);
        self->state = QxtDiscoverableService::Registered;
        emit self->qxt_p().registered();
    } else {
        self->state = QxtDiscoverableService::Unknown;
        emit self->qxt_p().registrationError((QxtDiscoverableService::ErrorCode)errCode);
    }
}

#ifdef Q_OS_WIN
void QxtDiscoverableServicePrivate::resolveServiceCallback(DNSServiceRef service, DNSServiceFlags flags, quint32 iface,
       DNSServiceErrorType errCode, const char* fullname, const char* host, quint16 port,
       quint16 txtLen, const char* txt, void* context)
#else
void QxtDiscoverableServicePrivate::resolveServiceCallback(DNSServiceRef service, DNSServiceFlags flags, quint32 iface,
        DNSServiceErrorType errCode, const char* fullname, const char* host, quint16 port, quint16 txtLen,
        const unsigned char* txt, void* context)
#endif
{
    Q_UNUSED(service);
    Q_UNUSED(txtLen);
    Q_UNUSED(txt);
    Q_UNUSED(flags);
    QxtDiscoverableServicePrivate* self = reinterpret_cast<QxtDiscoverableServicePrivate*>(context);
    QxtDiscoverableService* pub = &self->qxt_p();
    if(errCode == kDNSServiceErr_NoError) {
        QxtDiscoverableServiceName name(fullname);
        pub->setServiceName(name.serviceName());
        pub->setDomain(name.domain());
	pub->setHost(host);
	pub->setPort(qFromBigEndian(port));
	self->iface = iface;
        emit pub->resolved(fullname);
    } else {
        self->state = QxtDiscoverableService::Unknown;
        emit pub->resolveError((QxtDiscoverableService::ErrorCode)errCode);
    }
}

void QxtDiscoverableServicePrivate::socketData()
{
    DNSServiceProcessResult(service);
}

/**
 * Constructs a QxtDiscoverableService object using the specified service type.
 *
 * The service type may be a plain type name or it may be provided in the standard format
 * specified by the Zeroconf specification.
 *
 * The service name will be automatically generated based on the system's hostname.
 */
QxtDiscoverableService::QxtDiscoverableService(const QString& serviceType, QObject* parent)
: QObject(parent), QxtDiscoverableServiceName(QString(), serviceType, QString())
{
    QXT_INIT_PRIVATE(QxtDiscoverableService);
    qxt_zeroconf_parse_subtypes(&qxt_d(), serviceType.toUtf8());
}

/**
 * Constructs a QxtDiscoverableService object using the specified service type.
 *
 * The service type may be a plain type name or it may be provided in the standard format
 * specified by the Zeroconf specification.
 *
 * If the specified service name is already in use, it will be updated with a number to
 * make it unique.
 */
QxtDiscoverableService::QxtDiscoverableService(const QString& serviceType, const QString& serviceName, QObject* parent)
: QObject(parent), QxtDiscoverableServiceName(serviceName, serviceType, QString())
{
    QXT_INIT_PRIVATE(QxtDiscoverableService);
    qxt_zeroconf_parse_subtypes(&qxt_d(), serviceType.toUtf8());
}

/**
 * Destroys the QxtDiscoverableService.
 *
 * TIf registered, the service will be unregistered.
 */
QxtDiscoverableService::~QxtDiscoverableService()
{
    if(state() == Registered || state() == Resolved)
        releaseService();
}

/**
 * Returns the current state of the service.
 */
QxtDiscoverableService::State QxtDiscoverableService::state() const {
    return qxt_d().state;
}

/**
 * Returns a list of all subtypes known for the service.
 *
 * When discovering a service, only subtypes that were included in the service
 * discovery request will be included in this list.
 *
 * \sa setServiceSubTypes
 * \sa addServiceSubType
 * \sa removeServiceSubType
 * \sa hasServiceSubType
 */
QStringList QxtDiscoverableService::serviceSubTypes() const
{
    return qxt_d().serviceSubTypes;
}

/**
 * Sets the list of subtypes for this service.
 *
 * \sa serviceSubTypes
 * \sa addServiceSubType
 * \sa removeServiceSubType
 * \sa hasServiceSubType
 */
void QxtDiscoverableService::setServiceSubTypes(const QStringList& subtypes)
{
    if(state() != Unknown)
        qWarning() << "QxtDiscoverableService: Setting service subtypes while not in Unknown state has no effect";
    qxt_d().serviceSubTypes = subtypes;
}

/**
 * Adds a subtype to the service.
 *
 * \sa serviceSubTypes
 * \sa setServiceSubTypes
 * \sa removeServiceSubType
 * \sa hasServiceSubType
 */
void QxtDiscoverableService::addServiceSubType(const QString& subtype)
{
    if(state() != Unknown)
        qWarning() << "QxtDiscoverableService: Setting service subtypes while not in Unknown state has no effect";
    qxt_d().serviceSubTypes << subtype;
}

/**
 * Removes a subtype from the service.
 *
 * \sa serviceSubTypes
 * \sa setServiceSubTypes
 * \sa addServiceSubType
 * \sa hasServiceSubType
 */
void QxtDiscoverableService::removeServiceSubType(const QString& subtype)
{
    if(state() != Unknown)
        qWarning() << "QxtDiscoverableService: Setting service subtypes while not in Unknown state has no effect";
    qxt_d().serviceSubTypes.removeAll(subtype);
}

/**
 * Tests to see if the specified service is available and known for the service.
 *
 * When discovering a service, only subtypes that were included in the service
 * discovery request will be available.
 *
 * \sa serviceSubTypes
 * \sa setServiceSubTypes
 * \sa addServiceSubType
 * \sa removeServiceSubType
 */
bool QxtDiscoverableService::hasServiceSubType(const QString& subtype)
{
    return qxt_d().serviceSubTypes.contains(subtype);
}

/**
 * Returns the port number used for connecting to the service.
 *
 * \sa setPort
 */
quint16 QxtDiscoverableService::port() const
{
    return qFromBigEndian(qxt_d().port);
}

/**
 * Sets the port number used for connecting to the service.
 *
 * When registering a service with a port number of 0 (the default), the service will not be found when browsing,
 * but the service name will be marked as reserved.
 *
 * Setting the port is only meaningful when registering a service.
 *
 * \sa port
 */
void QxtDiscoverableService::setPort(quint16 port)
{
    qxt_d().port = qToBigEndian(port);
}

/**
 * Attempts to register the service on the local network.
 *
 * If noAutoRename is set to true, registration will fail if another service of the same service type
 * is already registered with the same service name. Otherwise, the service name will be updated with
 * a number to make it unique.
 *
 * \sa registered
 * \sa registrationError
 */
void QxtDiscoverableService::registerService(bool noAutoRename)
{
    if(state() != Unknown) {
        qWarning() << "QxtDiscoverableService: Cannot register service while not in Unknown state";
        emit registrationError(0);
        return;
    }

    QStringList subtypes = qxt_d().serviceSubTypes;
    subtypes.prepend(fullServiceType());

    DNSServiceErrorType err;
    err = DNSServiceRegister(&(qxt_d().service),
                             noAutoRename ? kDNSServiceFlagsNoAutoRename : 0,
                             qxt_d().iface,
                             serviceName().isEmpty() ? 0 : serviceName().toUtf8().constData(),
                             subtypes.join(",_").toUtf8().constData(),
                             domain().isEmpty() ? 0 : domain().toUtf8().constData(),
                             host().isEmpty() ? 0 : host().toUtf8().constData(),
                             qxt_d().port,
                             1, // must include null terminator
                             "",
                             QxtDiscoverableServicePrivate::registerServiceCallback,
                             &qxt_d());
    if(err != kDNSServiceErr_NoError) {
        qxt_d().state = Unknown;
        emit registrationError(err);
    } else {
        qxt_d().state = Registering;
        qxt_d().notifier = new QSocketNotifier(DNSServiceRefSockFD(qxt_d().service), QSocketNotifier::Read, this);
        QObject::connect(qxt_d().notifier, SIGNAL(activated(int)), &qxt_d(), SLOT(socketData()));
    }
}

/**
 * Attempts to resolve the service in order to determine the host and port necessary to establish a connection.
 *
 * If forceMulticast is set to true, QxtDiscoverableService will use a multicast request to resolve the service,
 * even if the host name appears to be a unicast address (that is, outside the local network).
 *
 * \sa resolved
 * \sa resolveError
 */
void QxtDiscoverableService::resolve(bool forceMulticast)
{
    if(state() != Unknown && state() != Found) {
        qWarning() << "QxtDiscoverableService: Cannot resolve service while not in Unknown or Found state";
        emit resolveError(0);
        return;
    }

    DNSServiceErrorType err;
    err = DNSServiceResolve(&(qxt_d().service),
                            (forceMulticast ? kDNSServiceFlagsForceMulticast : 0),
                            qxt_d().iface,
                            serviceName().toUtf8().constData(),
                            fullServiceType().constData(),
                            domain().toUtf8().constData(),
                            QxtDiscoverableServicePrivate::resolveServiceCallback,
                            &qxt_d());
    if(err != kDNSServiceErr_NoError) {
        qxt_d().state = Unknown;
        emit resolveError(err);
    } else {
        qxt_d().state = Resolving;
        qxt_d().notifier = new QSocketNotifier(DNSServiceRefSockFD(qxt_d().service), QSocketNotifier::Read, this);
        QObject::connect(qxt_d().notifier, SIGNAL(activated(int)), &qxt_d(), SLOT(socketData()));
    }
}

/**
 * Releases the service.
 *
 * If the service is registered, it will be unregistered. Any outstanding resolve attempt will be aborted.
 */
void QxtDiscoverableService::releaseService()
{
    if(state() != Registered && state() != Resolved) {
        qWarning() << "QxtDiscoverableService: Attempting to unregister an unresolved, unregistered service";
    } else {
        DNSServiceRefDeallocate(qxt_d().service);
        qxt_d().notifier->deleteLater();
    }
}

/**
 * \fn registered()
 *
 * This signal is emitted after a call to registerService() when the service has been successfully registered
 * on the network. The service name may have been updated to ensure uniqueness.
 *
 * \sa registerService
 * \sa registrationError
 */

/**
 * \fn registrationError(int code)
 *
 * This signal is emitted after a call to registerService() when the service cannot be registered. This could
 * be because the service name is already in use and noAutoRename was requested, or because the mDNSResponder
 * or Avahi daemon on the local machine could not be contacted.
 *
 * \sa registerService
 * \sa registered
 */

/**
 * \fn resolved(const QByteArray& domainName)
 *
 * This signal is emitted after a call to resolve() when a service matching the requested parameters is found.
 * domainName contains the complete domain name of the resolved service. The host name, port, and socket type
 * will be updated to match the connection parameters announced by the service.
 *
 * \sa resolve
 * \sa resolveError
 */

/**
 * \fn resolveError(int code)
 *
 * This signal is emitted after a call to resolve() if an error occurs while attempting to resolve the service.
 *
 * \sa resolve
 * \sa resolveError
 */
