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
#include "qxtslotjob_p.h"

/*!
\class QxtSlotJob

\inmodule QxtCore

\brief The QxtSlotJob class executes an arbitrary slot on a QThread.

\warning It is essential to understand that the QObject you pass is not safe to use untill done();  is emitted or result() or join() is called.
*/

/*!
execute \a slot from \a precv  on  \a thread  detached
returns a QFuture which offers the functions required to get the result.
\code
    QxtFuture f= QxtSlotJob::detach(&thread,&q,SLOT(exec(QString)),Q_ARG(QString, "select NOW();"));
\endcode

\warning keep your hands of \a recv until you called QFuture::result();
*/
QxtFuture QxtSlotJob::detach(QThread * thread, QObject* recv, const char* slot,
                             QGenericArgument p1,
                             QGenericArgument p2,
                             QGenericArgument p3,
                             QGenericArgument p4,
                             QGenericArgument p5,
                             QGenericArgument p6,
                             QGenericArgument p7,
                             QGenericArgument p8,
                             QGenericArgument p9,
                             QGenericArgument p10)
{
    QxtSlotJob * p = new  QxtSlotJob(recv, slot, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10);
    connect(p, SIGNAL(done()), p, SLOT(deleteLater()));
    return p->exec(thread);
}
/*!
Construct a new Job Object that will run \a slot from \a precv with the specified arguments
*/
QxtSlotJob::QxtSlotJob(QObject* recv, const char* slot,
                       QGenericArgument p1,
                       QGenericArgument p2,
                       QGenericArgument p3,
                       QGenericArgument p4,
                       QGenericArgument p5,
                       QGenericArgument p6,
                       QGenericArgument p7,
                       QGenericArgument p8,
                       QGenericArgument p9,
                       QGenericArgument p10)
{
    qxt_d().f = QxtMetaObject::bind(recv, slot, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10);
    qxt_d().receiver = recv;
    qxt_d().orginalthread = QThread::currentThread();

    connect(this, SIGNAL(done()), this, SLOT(pdone()));
}
/*!
asks for the result of the execution.
This calls QxtJob::join()  means it will _block_  the current thread untill the Slot has finished execution
*/
QVariant QxtSlotJob::result()
{
    join();
    return qxt_d().r;
}
/*!
execute this job on \a thread
\warning keep your hands of the Object you passed until you called result() or join()
*/
QxtFuture QxtSlotJob::exec(QThread *thread)
{
    qxt_d().receiver->moveToThread(thread);
    QxtJob::exec(thread);
    return QxtFuture(this);
}

void QxtSlotJob::run()
{
    qxt_d().r = qVariantFromValue(qxt_d().f->invoke());
    qxt_d().receiver->moveToThread(qxt_d().orginalthread);
}


void QxtSlotJob::pdone()
{
    emit(done(qxt_d().r));
}










/*!
\class QxtFuture

\inmodule QxtCore

\brief The QxtFuture class provides a reference to a future result of a QxtSlotJob

*/

QxtFuture::QxtFuture(const QxtFuture& other): QObject()
{
    job = other.job;
    connect(job, SIGNAL(done()), this, SIGNAL(done()));
    connect(job, SIGNAL(done(QVariant)), this, SIGNAL(done(QVariant)));
    waiter = new QxtSignalWaiter(job, SIGNAL(done()));
}

QxtFuture::QxtFuture(QxtSlotJob* j): QObject()
{
    job = j;
    connect(job, SIGNAL(done()), this, SIGNAL(done()));
    connect(job, SIGNAL(done(QVariant)), this, SIGNAL(done(QVariant)));
    waiter = new QxtSignalWaiter(job, SIGNAL(done()));
}

QxtFuture::~QxtFuture()
{
    delete waiter;
}
/*!
asks for the result of the execution.
This calls QxtJob::join()  means it will _block_  the current thread untill the Slot has finished execution
*/
QVariant QxtFuture::joinedResult()
{
    return job->result();
}



/*!
asks for the result of the execution.
waits until the done() signal occurred
or  return a QVariant() if the timout ocures earlier
This uses QxtSignalWaiter so it will _not_ block your current thread.
\warning this function is not reentrant. You have been warned

*/



QVariant QxtFuture::delayedResult(int msec)
{
    if (!waiter->wait(msec, false))
        return QVariant();
    return job->result();
}








