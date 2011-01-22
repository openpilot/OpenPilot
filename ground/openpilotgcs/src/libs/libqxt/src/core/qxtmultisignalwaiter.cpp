/****************************************************************************
 **
 ** Copyright (C) Qxt Foundation. Some rights reserved.
 **
 ** This file is part of the QxtCore module of the Qxt library.
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

/*!
\class QxtMultiSignalWaiter

\inmodule QxtCore

\brief The QxtMultiSignalWaiter class blocks and processes events until a group of signals is emitted

Code written in a synchronous style will sometimes need to block until any of several conditions are met,
or until all of a set of conditions are met. This class allows a group of signals to be defined and waited
upon in a simple AND or OR fashion. More complex Boolean relationships can be written by connecting
together multiple QxtSignalGroup objects and waiting for all or any of these groups to emit one of
their signals.

\bold {Note:}
QxtMultiSignalWaiter is subject to the same reentrancy problems as QxtSignalWaiter.

\sa QxtSignalWaiter
*/

#include "qxtmultisignalwaiter.h"
#include "qxtsignalwaiter.h"

/*!
 * Constructs a QxtMultiSignalWaiter with the specified \a parent.
 */
QxtMultiSignalWaiter::QxtMultiSignalWaiter(QObject* parent) : QxtSignalGroup(parent)
{
    /* initializers only */
}

/*!
 * Destructs the multi signal waiter.
 */
QxtMultiSignalWaiter::~QxtMultiSignalWaiter()
{
    /* no-op */
}

/*!
 * Blocks the current function by processing events from the event queue according to
 * \a flags until any of the signals in the group are emitted.
 * If \a msec is not \c -1, waitForAny() will return before a signal is emitted if the
 * specified  number of milliseconds have elapsed. Returns \c true if a signal was
 * caught, or \c false if the timeout elapsed.
 * Note that waitForAny() may continue to block after a signal is emitted or the
 * timeout elapses; the function only guarantees that it will not return BEFORE
 * one of these conditions has occurred. This function is not reentrant.
 *
 * \sa QxtSignalGroup::addSignal(), QxtSignalGroup::firstSignalReceived()
 */
bool QxtMultiSignalWaiter::waitForAny(int msec, QEventLoop::ProcessEventsFlags flags)
{
    if (hasReceivedFirstSignal()) return true;
    return QxtSignalWaiter::wait(this, SIGNAL(firstSignalReceived()), msec, flags);
}

/*!
 * Blocks the current function by processing events from the event queue according to
 * \a flags until all of the signals in the group have been emitted. If \a msec is not
 * \c -1, waitForAll() will return before all of the signals are emitted if the specified
 * number of milliseconds have elapsed. Returns \c true if each signal was caught at
 * least once, or \c false if the timeout elapsed. Note that waitForAll() may continue
 * to block after the last signal is emitted or the timeout elapses; the function only
 * guarantees that it will not return BEFORE one of these conditions has occurred.
 * This function is not reentrant.
 *
 * \sa QxtSignalGroup::addSignal(), QxtSignalGroup::allSignalsReceived()
 */
bool QxtMultiSignalWaiter::waitForAll(int msec, QEventLoop::ProcessEventsFlags flags)
{
    if (hasReceivedAllSignals()) return true;
    return QxtSignalWaiter::wait(this, SIGNAL(allSignalsReceived()), msec, flags);
}

