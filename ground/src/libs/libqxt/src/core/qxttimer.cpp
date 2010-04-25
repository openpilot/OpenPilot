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
#include "qxttimer.h"
#include "qxtmetaobject.h"
#include <QTimerEvent>
#include <QPointer>

/*!
    \class QxtTimer
    \inmodule QxtCore
    \brief The QxtTimer class extends QTimer.

    QxtTimer extends QTimer with capability to call a slot after with given
    parameters after a given time interval.

    Example usage:
    \code
    QxtTimer::singleShot(500, widget, SLOT(setWindowTitle(QString)), QString("Window Title"));
    \endcode
 */

class QxtSingleShotTimer : public QObject
{
public:
    QxtSingleShotTimer(int msec, QObject* receiver, const char* member, const QVariantList& args);

protected:
    void timerEvent(QTimerEvent* event);

private:
    QPointer<QObject> receiver;
    const char* member;
    QVariantList args;
    int timerId;
};

QxtSingleShotTimer::QxtSingleShotTimer(int msec, QObject* receiver, const char* member, const QVariantList& args) :
    receiver(receiver), member(member), args(args), timerId(-1)
{
    timerId = startTimer(msec);
}

void QxtSingleShotTimer::timerEvent(QTimerEvent* event)
{
    if (event->timerId() == timerId)
    {
        QxtMetaObject::invokeMethod(receiver, member, args.at(0), args.at(1),
                                    args.at(2), args.at(3), args.at(4), args.at(5), 
                                    args.at(6), args.at(7), args.at(8), args.at(9));
        deleteLater();
    }
}

/*!
    Constructs a new QxtTimer with \a parent.
 */
QxtTimer::QxtTimer(QObject* parent) : QTimer(parent)
{
}

/*!
    Destructs the timer.
 */
QxtTimer::~QxtTimer()
{
}

/*!
    This static function calls a slot with given parameters after a given time interval.

    It is very convenient to use this function because you do not need to bother with a timerEvent or create a local QTimer object.

    You can pass up to ten arguments (\a arg0, \a arg1, \a arg2, \a arg3, \a arg4, \a arg5, \a arg6, \a arg7, \a arg8 and \a arg9).

    The \a receiver is the receiving object and the \a member is the slot. The time interval is \a msec milliseconds.
 */
void QxtTimer::singleShot(int msec, QObject* receiver, const char* member, const QVariant& arg0,
                          const QVariant& arg1, const QVariant& arg2, const QVariant& arg3,
                          const QVariant& arg4, const QVariant& arg5, const QVariant& arg6,
                          const QVariant& arg7, const QVariant& arg8, const QVariant& arg9)
{
    if (receiver && member)
    {
        QVariantList args;
        args << arg0 << arg1 << arg2 << arg3 << arg4 << arg5 << arg6 << arg7 << arg8 << arg9;
        (void) new QxtSingleShotTimer(msec, receiver, member, args);
    }
}
