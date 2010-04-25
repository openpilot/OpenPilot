#include "qxtdiscoverableservicename.h"
#include <dns_sd.h>
#include <QMutex>
#include <QMutexLocker>
#include <QPointer>


#ifdef USE_AVAHI
#include <qxtmdns_avahi.h>
#else
#include <qxtmdns_bonjour.h>
#endif

class QxtDiscoverableServiceNamePrivate : public QxtPrivate<QxtDiscoverableServiceName>
{
public:
	QXT_DECLARE_PUBLIC(QxtDiscoverableServiceName)
	QString name;
	QString domain;
	QAbstractSocket::SocketType socketType;
	QString serviceType;
	QString host;

	//mDNS stuff
	static int id;
	static QMutex idMutex;
	static QHash<int, QPointer<QxtMDNS> > lookups;
};

int QxtDiscoverableServiceNamePrivate::id = 0;
QMutex QxtDiscoverableServiceNamePrivate::idMutex;
QHash<int, QPointer<QxtMDNS> > QxtDiscoverableServiceNamePrivate::lookups;

QxtDiscoverableServiceName::QxtDiscoverableServiceName()
{
    QXT_INIT_PRIVATE(QxtDiscoverableServiceName);
    qxt_d().socketType = QAbstractSocket::UnknownSocketType;
}

QxtDiscoverableServiceName::QxtDiscoverableServiceName(const QByteArray& domainName)
{
	QXT_INIT_PRIVATE(QxtDiscoverableServiceName);
	// TODO: parse domainName
}

QxtDiscoverableServiceName::QxtDiscoverableServiceName(const QString& name, const QString& serviceType, const QString& domain,
        QAbstractSocket::SocketType socketType)
{
	QXT_INIT_PRIVATE(QxtDiscoverableServiceName);
	qxt_d().name = name;
	setServiceType(serviceType);
	qxt_d().domain = domain;
	if (socketType != QAbstractSocket::UnknownSocketType)
		qxt_d().socketType = socketType;
}

QString QxtDiscoverableServiceName::serviceName() const
{
	return qxt_d().name;
}

void QxtDiscoverableServiceName::setServiceName(const QString& name)
{
	qxt_d().name = name;
}

QAbstractSocket::SocketType QxtDiscoverableServiceName::socketType() const
{
	return qxt_d().socketType;
}

void QxtDiscoverableServiceName::setSocketType(QAbstractSocket::SocketType type)
{
	qxt_d().socketType = type;
}

QString QxtDiscoverableServiceName::serviceType() const
{
	return qxt_d().serviceType;
}

void QxtDiscoverableServiceName::setServiceType(const QString& type)
{
	QList<QByteArray> subtypes = type.toUtf8().split(',');
	QList<QByteArray> rt = subtypes[0].split('.');
	setSocketType(QAbstractSocket::TcpSocket);

	if (rt.count() != 2)
	{
		qxt_d().serviceType = subtypes[0];
	}
	else
	{
		if (rt.at(0)[0] == '_')
		{
			qxt_d().serviceType = rt[0].mid(1);
		}
		else
		{
			qxt_d().serviceType = rt[0];
		}
		if (rt[1] == "_udp")
		{
			setSocketType(QAbstractSocket::UdpSocket);
		}
		else if (rt[1] != "_tcp")
		{
			setSocketType(QAbstractSocket::UnknownSocketType);
		}
	}
}

QString QxtDiscoverableServiceName::domain() const
{
	return qxt_d().domain;
}

void QxtDiscoverableServiceName::setDomain(const QString& domain)
{
	qxt_d().domain = domain;
}

QString QxtDiscoverableServiceName::host() const
{
	return qxt_d().host;
}

void QxtDiscoverableServiceName::setHost(const QString& host)
{
	qxt_d().host = host;
}

QByteArray QxtDiscoverableServiceName::fullServiceType() const
{
	QString regtype = "_" + qxt_d().serviceType + "._";
	if (qxt_d().socketType == QAbstractSocket::TcpSocket)
		regtype += "tcp";
	else
		regtype += "udp";
	return regtype.toUtf8();
}

QByteArray QxtDiscoverableServiceName::escapedFullDomainName() const
{
	char buffer[kDNSServiceMaxDomainName] = "";
	int err = DNSServiceConstructFullName(buffer,
	                                      qxt_d().name.isEmpty() ? 0 : qxt_d().name.toUtf8().constData(),
	                                      fullServiceType().constData(),
	                                      domain().toUtf8().constData());
	if (err)
		return QByteArray(); // error
	return buffer;
}

int QxtDiscoverableServiceName::lookupHost(const QString name, QObject* receiver, const char* member)
{
	QMutexLocker locker(&QxtDiscoverableServiceNamePrivate::idMutex);
	QxtDiscoverableServiceNamePrivate::id++;
	QxtMDNS* md = new QxtMDNS(QxtDiscoverableServiceNamePrivate::id);
	md->doLookup(name, receiver, member);
	QxtDiscoverableServiceNamePrivate::lookups[QxtDiscoverableServiceNamePrivate::id] = md;
	return QxtDiscoverableServiceNamePrivate::id;
}

void QxtDiscoverableServiceName::abortHostLookup(int id)
{
	QMutexLocker locker(&QxtDiscoverableServiceNamePrivate::idMutex);
	if (QxtDiscoverableServiceNamePrivate::lookups.contains(id))
	{
		if (QxtDiscoverableServiceNamePrivate::lookups[id].isNull())
			QxtDiscoverableServiceNamePrivate::lookups.remove(id);
		else
			QxtDiscoverableServiceNamePrivate::lookups.take(id)->cancelLookup();
	}
}
