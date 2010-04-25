/****************************************************************************
 **
 ** Copyright (C) Qxt Foundation. Some rights reserved.
 **
 ** This file is part of the QxtZeroconf module of the Qxt library.
 **
 ** This library is free software; you can redistribute it and/or modify it
 ** under the terms of the Common Public License, version 1.0, as published
 ** by IBM, and/or under the terms of the GNU Lesser General Public License,
 ** version 2.1, as published by the Free Software Foundation.
 **
 ** This file is provided "AS IS", without WARRANTIES OR CONDITIONS OF ANY
 ** KIND, EITHER EXPRESS OR IMPLIED INCLUDING, WITHOUT LIMITATION, ANY
 ** WARRANTIES OR CONDITIONS OF TITLE, NON-INFRINGEMENT, MERCHANTABILITY OR
 ** FITNESS FOR A PARTICULAR PURPOSE.
 **
 ** You should have received a copy of the CPL and the LGPL along with this
 ** file. See the LICENSE file and the cpl1.0.txt/lgpl-2.1.txt files
 ** included with the source distribution for more information.
 ** If you did not receive a copy of the licenses, contact the Qxt Foundation.
 **
 ** <http://libqxt.org>  <foundation@libqxt.org>
 **
 ****************************************************************************/

#ifndef QXTMDNS_P_H
#define QXTMDNS_P_H

#include "qxtmdns_avahi.h"
#include <QHostAddress>
#include <QHostInfo>
#include <avahi-client/client.h>
#include <avahi-client/lookup.h>
#include <qxtavahipoll.h>

class QxtMDNSPrivate : public QxtPrivate<QxtMDNS>
{
public:
	QxtMDNSPrivate()
	{
	}

	static void avahiClientCallback(AvahiClient *s, AvahiClientState state, void *userdata);
	static void avahiRecordBrowserCallback(AvahiRecordBrowser *b, AvahiIfIndex interface, AvahiProtocol protocol, AvahiBrowserEvent event, const char *name, uint16_t clazz, uint16_t type, const void *rdata, size_t size, AvahiLookupResultFlags flags, void *userdata);

	QHostInfo info;
	QList<QHostAddress> addresses;

	QString name;
	QObject* receiver;
	QByteArray member;
	AvahiClient* client;
	AvahiRecordBrowser* recordbrowser;
	bool sent;
	int id;
};


#endif //#ifndef QXTMDNS_H
