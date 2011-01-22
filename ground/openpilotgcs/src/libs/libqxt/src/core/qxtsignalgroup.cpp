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
\class QxtSignalGroup

\inmodule QxtCore

\brief The QxtSignalGroup class groups signals together in a Boolean fashion

When carrying out multiple tasks in parallel, it can be useful to know when any or all of the tasks
have emitted a signal. This class allows a group of signals to be defined and re-signalled in a
simple AND or OR fashion. More complex Boolean relationships can be written by connecting
together multiple QxtSignalGroup objects.

For a simple example, suppose you have four QHttp requests pending and you want to know when the
last request has completed. A QxtSignalGroup defined like this will emit its allSignalsReceived()
signal after each request has emitted its done() signal.
\code
    QxtSignalGroup* group = new QxtSignalGroup;
    for(int i = 0; i < 4; i++) {
        group->addSignal(http[i], SIGNAL(done()));
    }
\endcode

For a more complex example, suppose you have two such batches of requests, and you want a signal
to be emitted when the first batch is completed. Create two QxtSignalGroup objects (perhaps
group1 and group2) and add their signals to a third group. This third group will emit its
firstSignalReceived() signal when the first batch is completed and its allSignalsReceived()
signal when all batches are finished.
\code
    QxtSignalGroup* batches = new QxtSignalGroup;
    batches->addSignal(group1, SIGNAL(allSignalsReceived()));
    batches->addSignal(group2, SIGNAL(allSignalsReceived()));
\endcode
*/
#include "qxtsignalgroup.h"
#include <QVector>
#include <QMetaObject>
#include <QtDebug>

class QxtSignalGroupPrivate : public QObject, public QxtPrivate<QxtSignalGroup>
{
public:
    QXT_DECLARE_PUBLIC(QxtSignalGroup)
    QxtSignalGroupPrivate() : QObject(0)
    {
        // since we don't have a metaobject of our own due to not
        // using the Q_OBJECT macro, we need to find methodOffset
        // on our own.
        baseSignal = QObject::staticMetaObject.methodCount();
        emitCount = disconnectCount = 0;
    }

    QVector<bool> emittedSignals;
    int baseSignal, emitCount, disconnectCount;

protected:
    int qt_metacall(QMetaObject::Call _c, int _id, void **_a)
    {
        Q_UNUSED(_c);
        Q_UNUSED(_a);
        // We don't care about QObject's methods, so skip them
        _id -= baseSignal;
        int ct = emittedSignals.count();    // cached for slight performance gain
        if (_id < 0 || _id > ct) return _id;
        bool& state = emittedSignals[_id];  // more performance caching
        if (!state)
        {
            if (emitCount == 0)
                qxt_p().firstSignalReceived();
            emitCount++;
            state = true;
            if (emitCount + disconnectCount == ct)
                qxt_p().allSignalsReceived();
        }
        return _id;
    }
};

/*!
 * Constructs a QxtSignalWaiter with the specified \a parent.
 */
QxtSignalGroup::QxtSignalGroup(QObject* parent) : QObject(parent)
{
    QXT_INIT_PRIVATE(QxtSignalGroup);
}

/*!
 * Returns \c true if at least one attached signal has been emitted since the last reset().
 */
bool QxtSignalGroup::hasReceivedFirstSignal() const
{
    return qxt_d().emitCount > 0;
}

/*!
 * Returns \c true if every attached signal has been emitted at least once since the last reset().
 */
bool QxtSignalGroup::hasReceivedAllSignals() const
{
    return (qxt_d().emitCount + qxt_d().disconnectCount) >= qxt_d().emittedSignals.count();
}

/*!
 * Add a signal from \a sender with signature \a sig to the group.
 */
void QxtSignalGroup::addSignal(QObject* sender, const char* sig)
{
    int signalID = sender->metaObject()->indexOfSignal(QMetaObject::normalizedSignature(sig + 1));
    if (signalID < 0)
    {
        qWarning() << "QxtSignalGroup::addSignal: no such signal" << sig;
    }
    else
    {
        QMetaObject::connect(sender, signalID, &(qxt_d()), qxt_d().emittedSignals.count() + qxt_d().baseSignal);
        qxt_d().emittedSignals.append(false);
    }
}

/*!
 * Remove a signal from \a sender with signature \a sig from the group.
 */
void QxtSignalGroup::removeSignal(QObject* sender, const char* sig)
{
    if (QObject::disconnect(sender, sig, &(qxt_d()), 0))
        qxt_d().disconnectCount++;
}

/*!
 * Reset the signal tracking, that is, after calling reset() no signals are considered to have been caught.
 */
void QxtSignalGroup::reset()
{
    qxt_d().emittedSignals.fill(false);
    qxt_d().emitCount = 0;
}

/*!
 * Removes all signals from the group and resets the signal tracking.
 */
void QxtSignalGroup::clear()
{
    qxt_d().emittedSignals.clear();
    qxt_d().emitCount = 0;
    qxt_d().disconnectCount = 0;
}

/*!
 * \fn void QxtSignalGroup::firstSignalReceived();
 * This signal is emitted the first time a signal in the group is emitted.
 * After this signal is emitted once, you must call reset() before it can be emitted again.
 */

/*!
 * \fn void QxtSignalGroup::allSignalsReceived();
 * This signal is emitted after every signal in the group has been emitted at least once.
 * After this signal is emitted once, you must call reset() before it can be emitted again.
 */
