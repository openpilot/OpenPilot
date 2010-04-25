#include "qxtservicebrowser.h"
#include "qxtservicebrowser_p.h"
#include "qxtdiscoverableservice_p.h"
#include <QSocketNotifier>

/*!
\class QxtServiceBrowser

\inmodule QxtZeroconf

\brief The QxtServiceBrowser class locates Zeroconf-enabled services on the network

QxtServiceBrowser is a tool for locating Zeroconf-enabled services on the local network. When starting
to browse, all existing services reachable on the local network matching the specified search parameters
will be announced with the serviceAdded() signal. Afterward, while browsing, new services apearing on
the network will be announced with the same signal, and services that are closed will be announced with
the serviceRemoved() signal.

Services on the network are identified by a service type and may be further disambiguated by service
subtypes. If any service subtypes are added to the QxtServiceBrowser, only services offering the
specified subtypes will be found. Use the properties inherited from QxtDiscoverableServiceName to
configure the search parameters.

For more information about Zeroconf, see the documentation available on www.zeroconf.org.

\sa QxtDiscoverableService
\sa QxtDiscoverableServiceName
*/

/**
 * Constructs a QxtServiceBrowser that will search for services with the specified type.
 * The service type may be a plain type name, which will default to searching for a UDP
 * service, or it may be provided in the standard format specified by the Zeroconf
 * specification.
 */
QxtServiceBrowser::QxtServiceBrowser(const QString& serviceType, QObject* parent)
: QObject(parent), QxtDiscoverableServiceName(QString(), serviceType, QString())
{
    QXT_INIT_PRIVATE(QxtServiceBrowser);
    qxt_zeroconf_parse_subtypes(&qxt_d(), serviceType.toUtf8());
}

/**
 * Constructs a QxtServiceBrowser that will search for services with the specified type
 * and socket type.
 */
QxtServiceBrowser::QxtServiceBrowser(const QString& serviceType, QAbstractSocket::SocketType socketType, QObject* parent)
: QObject(parent), QxtDiscoverableServiceName(QString(), serviceType, QString(), socketType)
{
    QXT_INIT_PRIVATE(QxtServiceBrowser);
    qxt_zeroconf_parse_subtypes(&qxt_d(), serviceType.toUtf8());
}

QxtServiceBrowser::~QxtServiceBrowser() {
    if(isBrowsing())
        stopBrowsing();
}

bool QxtServiceBrowser::isBrowsing() const
{
    return bool(qxt_d().notifier);
}

void QxtServiceBrowser::browse(/* int iface */)
{
    QStringList subtypes = qxt_d().serviceSubTypes;
    subtypes.prepend(fullServiceType());

    DNSServiceErrorType err;
    err = DNSServiceBrowse(&(qxt_d().service),
                           0,
                           qxt_d().iface,
                           subtypes.join(",_").toUtf8().constData(),
                           domain().isEmpty() ? 0 : domain().toUtf8().constData(),
                           QxtServiceBrowserPrivate::browseServiceCallback,
                           &qxt_d());
    if(err) {
        emit browsingFailed(err);
    } else {
        qxt_d().notifier = new QSocketNotifier(DNSServiceRefSockFD(qxt_d().service), QSocketNotifier::Read, this);
        QObject::connect(qxt_d().notifier, SIGNAL(activated(int)), &qxt_d(), SLOT(socketData()));
    }
}

void QxtServiceBrowser::stopBrowsing()
{
    if(qxt_d().notifier) {
        DNSServiceRefDeallocate(qxt_d().service);
        qxt_d().notifier->deleteLater();
    }
    qxt_d().notifier = 0;
}

QStringList QxtServiceBrowser::serviceSubTypes() const
{
    return qxt_d().serviceSubTypes;
}

void QxtServiceBrowser::setServiceSubTypes(const QStringList& subtypes)
{
    if(isBrowsing())
        qWarning() << "QxtServiceBrowser: Setting service subtypes while browsing has no effect";
    qxt_d().serviceSubTypes = subtypes;
}

void QxtServiceBrowser::addServiceSubType(const QString& subtype)
{
    if(isBrowsing())
        qWarning() << "QxtServiceBrowser: Setting service subtypes while browsing has no effect";
    qxt_d().serviceSubTypes << subtype;
}

void QxtServiceBrowser::removeServiceSubType(const QString& subtype)
{
    if(isBrowsing())
        qWarning() << "QxtServiceBrowser: Setting service subtypes while browsing has no effect";
    qxt_d().serviceSubTypes.removeAll(subtype);
}

bool QxtServiceBrowser::hasServiceSubType(const QString& subtype)
{
    return qxt_d().serviceSubTypes.contains(subtype);
}

void QxtServiceBrowserPrivate::socketData()
{
    DNSServiceProcessResult(service);
}

void QxtServiceBrowserPrivate::browseServiceCallback(DNSServiceRef service, DNSServiceFlags flags, quint32 iface,
        DNSServiceErrorType errCode, const char* serviceName, const char* regtype, const char* replyDomain, void* context)
{
    Q_UNUSED(service);
    Q_UNUSED(iface);
    Q_UNUSED(regtype);
    QxtServiceBrowserPrivate* self = reinterpret_cast<QxtServiceBrowserPrivate*>(context);
    if(errCode == kDNSServiceErr_NoError) {
        if(flags & kDNSServiceFlagsAdd)
            emit self->qxt_p().serviceAdded(serviceName, replyDomain);
        else
            emit self->qxt_p().serviceRemoved(serviceName, replyDomain);
    } else {
        emit self->qxt_p().stopBrowsing();
        emit self->qxt_p().browsingFailed(errCode);
    }
}
