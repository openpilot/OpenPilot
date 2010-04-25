/****************************************************************************
 **
 ** Copyright (C) Qxt Foundation. Some rights reserved.
 **
 ** This file is part of the QxtWeb module of the Qxt library.
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

#ifndef QXTAVAHIPOLL_P_H
#define QXTAVAHIPOLL_P_H

#include <QObject>
#include <QTimer>
#include <QSocketNotifier>
#include <avahi-common/watch.h>

// Private QObject that does all the work

/**
	@author Chris Vickery <chrisinajar@gmail.com>
*/
class AvahiWatch : public QObject
{
	Q_OBJECT
public:
	AvahiWatch(int fd, AvahiWatchEvent event, AvahiWatchCallback callback);
	~AvahiWatch();
	void setEventType(AvahiWatchEvent event);
	AvahiWatchEvent lastEvent();

private slots:
	void activated(int);

private:
	QSocketNotifier* _notifier;
	int _fd;
	AvahiWatchEvent _event;
	AvahiWatchEvent _lastEvent;
	AvahiWatchCallback _callback;
};

class AvahiTimeout : public QObject
{
	Q_OBJECT
public:
	AvahiTimeout(const struct timeval *tv, AvahiTimeoutCallback callback);
	~AvahiTimeout();
	void updateTimeout(const struct timeval *tv);

private slots:
	void timeout();
private:
	QTimer _timer;
	AvahiTimeoutCallback _callback;
};

#endif
