#ifndef QXTMDNS_H
#define QXTMDNS_H

#include <QObject>
#include <QHostInfo>
#include <dns_sd.h>
#include <QSocketNotifier>

///@TODO Finish and test implementation of Bonjour mDNS, and remove the following warning.
// Sadly I can't test this because this part of the avahi Bonjour compatibility layer is broken on Ubuntu Linux version 8.04... -chrisinajar
#ifndef Q_CC_MSVC
#warning The Bonjour implementation is in no way tested, and is not fully implemented either. Try again later.
#endif

class QxtMDNS : public QObject
{
	Q_OBJECT
public:
	QxtMDNS(int id = -1, QObject* parent = 0);

	void doLookup(QString name, QObject * receiver, const char * member);
	void cancelLookup();

	static void DNSSD_API DNSServiceQueryRecordCallback(DNSServiceRef DNSServiceRef, DNSServiceFlags flags, uint32_t interfaceIndex, DNSServiceErrorType errorCode, const char *fullname, uint16_t rrtype, uint16_t rrclass, uint16_t rdlen, const void *rdata, uint32_t ttl, void *context);

	QHostInfo info;
	QList<QHostAddress> addresses;
	QString name;
	QObject* receiver;
	const char* member;
	DNSServiceRef ref;
	QSocketNotifier* notifier;
	int id;
public Q_SLOTS:
	void socketData();


};

#endif //#ifndef QXTMDNS_H
