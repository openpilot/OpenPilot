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

#include "qxtavahipoll.h"
#include "qxtavahipoll_p.h"

#include <QThread>

// declare the functions for the struct
// comments copied from watch.h so that I know what they do :P

/* Create a new watch for the specified file descriptor and for
 * the specified events. The API will call the callback function
 * whenever any of the events happens. */
AvahiWatch* qxtAvahiWatchNew(const AvahiPoll *api, int fd, AvahiWatchEvent event, AvahiWatchCallback callback, void *userdata);
/* Update the events to wait for. It is safe to call this function from an AvahiWatchCallback */
void qxtAvahiWatchUpdate(AvahiWatch *w, AvahiWatchEvent event);
/* Return the events that happened. It is safe to call this function from an AvahiWatchCallback  */
AvahiWatchEvent qxtAvahiWatchGetEvents(AvahiWatch *w);
/* Free a watch. It is safe to call this function from an AvahiWatchCallback */
void qxtAvahiWatchFree(AvahiWatch *w);
/* Set a wakeup time for the polling loop. The API will call the
callback function when the absolute time *tv is reached. If tv is
NULL, the timeout is disabled. After the timeout expired the
callback function will be called and the timeout is disabled. You
can reenable it by calling timeout_update()  */
AvahiTimeout* qxtAvahiTimeoutNew(const AvahiPoll *api, const struct timeval *tv, AvahiTimeoutCallback callback, void *userdata);
/* Update the absolute expiration time for a timeout, If tv is
 * NULL, the timeout is disabled. It is safe to call this function from an AvahiTimeoutCallback */
void qxtAvahiTimeoutUpdate(AvahiTimeout *, const struct timeval *tv);
/* Free a timeout. It is safe to call this function from an AvahiTimeoutCallback */
void qxtAvahiTimeoutFree(AvahiTimeout *t);

const AvahiPoll* qxtAvahiPoll(void)
{
	static const AvahiPoll avahiPoll =
	{
		NULL,
		qxtAvahiWatchNew,
		qxtAvahiWatchUpdate,
		qxtAvahiWatchGetEvents,
		qxtAvahiWatchFree,
		qxtAvahiTimeoutNew,
		qxtAvahiTimeoutUpdate,
		qxtAvahiTimeoutFree
	};
	return &avahiPoll;
}

AvahiWatch* qxtAvahiWatchNew(const AvahiPoll *api, int fd, AvahiWatchEvent event, AvahiWatchCallback callback, void *userdata)
{
	return new AvahiWatch(fd, event, callback);
}

void qxtAvahiWatchUpdate(AvahiWatch *w, AvahiWatchEvent event)
{
	w->setEventType(event);
}

AvahiWatchEvent qxtAvahiWatchGetEvents(AvahiWatch *w)
{
	return w->lastEvent();
}

void qxtAvahiWatchFree(AvahiWatch *w)
{
	if (w->thread() == QThread::currentThread())
		delete w;
	else
		w->deleteLater();
}

AvahiTimeout* qxtAvahiTimeoutNew(const AvahiPoll *api, const struct timeval *tv, AvahiTimeoutCallback callback, void *userdata)
{
	return new AvahiTimeout(tv, callback);
}

void qxtAvahiTimeoutUpdate(AvahiTimeout *t, const struct timeval *tv)
{
	t->updateTimeout(tv);
}

void qxtAvahiTimeoutFree(AvahiTimeout *t)
{
	if (t->thread() == QThread::currentThread())
		delete t;
	else
		t->deleteLater();
}

/* WATCH */
AvahiWatch::AvahiWatch(int fd, AvahiWatchEvent event, AvahiWatchCallback callback)
		: _notifier(0),
		_fd(fd),
		_event(event),
		_lastEvent(event),
		_callback(callback)
{
	setEventType(event);
}

AvahiWatch::~AvahiWatch()
{
}

void AvahiWatch::setEventType(AvahiWatchEvent event)
{
	if (_notifier != 0)
	{
		if (_notifier->thread() == QThread::currentThread())
			delete _notifier;
		else
			_notifier->deleteLater();
		_notifier = 0;
	}
	_event = event;
	switch (_event)
	{
		case AVAHI_WATCH_IN:
		{
			_notifier = new QSocketNotifier(_fd, QSocketNotifier::Read, this);
			connect(_notifier, SIGNAL(activated(int)), this, SLOT(activated(int)));
			break;
		}
		case AVAHI_WATCH_OUT:
		{
			_notifier = new QSocketNotifier(_fd, QSocketNotifier::Write, this);
			connect(_notifier, SIGNAL(activated(int)), this, SLOT(activated(int)));
			break;
		}
		default: //The constructor should only get in or out...
			qWarning("Bad event type passed to AvahiWatch constructor");
			break;
	}
}

void AvahiWatch::activated(int)
{
	_lastEvent = _event;
	_callback(this, _fd, _event, NULL);
}

AvahiWatchEvent AvahiWatch::lastEvent()
{
	return _lastEvent;
}

/* TIMEOUT */
AvahiTimeout::AvahiTimeout(const struct timeval *tv, AvahiTimeoutCallback callback)
		: _callback(callback)
{
	updateTimeout(tv);
}

AvahiTimeout::~AvahiTimeout()
{
}

void AvahiTimeout::updateTimeout(const struct timeval *tv)
{
	if(tv == 0)
	{
		_timer.stop();
		return;
	}
	int msecs = (tv->tv_sec * 1000) + (tv->tv_usec / 1000);
	if (msecs > 1)
		_timer.start(msecs);
	else
		_timer.stop();
}

void AvahiTimeout::timeout()
{
	_timer.stop();
	_callback(this, NULL);
}


