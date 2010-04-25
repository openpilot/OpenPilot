#include "qxtmdns_bonjour.h"
#include <qxtmetaobject.h>
#include <QDebug>
#include <QtEndian>

QxtMDNS::QxtMDNS(int id, QObject * parent)
		: QObject(parent),
          info(id)
{
}

void QxtMDNS::doLookup(QString n, QObject * r, const char * m)
{

	name = n;
	receiver = r;
	member = QxtMetaObject::methodName(m);
	DNSServiceErrorType err = DNSServiceQueryRecord(
	                              &ref,
	                              0,
	                              0,
	                              name.toAscii().constData(),
	                              kDNSServiceType_SRV,
	                              kDNSServiceClass_IN,
	                              QxtMDNS::DNSServiceQueryRecordCallback,
	                              this
	                          );
	if (err != kDNSServiceErr_NoError)
	{
		QHostInfo info(info.lookupId());
		info.setErrorString("Failed to initialize the Bonjour request.");
		QMetaObject::invokeMethod(receiver, member, Q_ARG(QHostInfo, info));
	}
	else
	{
		notifier = new QSocketNotifier(DNSServiceRefSockFD(ref), QSocketNotifier::Read, this);
		QObject::connect(notifier, SIGNAL(activated(int)), this, SLOT(socketData()));
	}
}

void QxtMDNS::DNSServiceQueryRecordCallback(DNSServiceRef DNSServiceRef, DNSServiceFlags flags, uint32_t interfaceIndex, DNSServiceErrorType errorCode, const char *fullname, uint16_t rrtype, uint16_t rrclass, uint16_t rdlen, const void *rdata, uint32_t ttl, void *context)
{
	QxtMDNS* md = static_cast<QxtMDNS*>(context);
	QHostInfo info(md->info.lookupId());
	uint32_t ip = qFromBigEndian(*static_cast<const uint32_t*>(rdata));
	info.setAddresses(QList<QHostAddress>() << QHostAddress(ip));
	QMetaObject::invokeMethod(md->receiver, md->member, Q_ARG(QHostInfo, info));
}

void QxtMDNS::socketData()
{
	DNSServiceProcessResult(ref);
}

void QxtMDNS::cancelLookup()
{
}
