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

/*
\class QxtJob

\inmodule QxtCore

\brief The QxtJob class executes a job on a QThread. Once or multiple times.

QxtJob allows easily starting jobs on different threads.
exec() will ask for the QThread to run the job on.
The Qthread needs an event loop. since version 4.4, QThread has
a non pure run function with a default event loop, allowing easy deployment of jobs.

\code
QThread thread;
thread.start();
LockJob().exec(&thread);
\endcode
*/


/*
\fn  void QxtJob::run()
This function is called by QxtJob.
reimplemented this function to do useful work.
Returning from this method will end the execution of the job.
*/

/*
\fn  void QxtJob::done()
This signal is emitted, when the run() function returns.
*/

#include "qxtjob_p.h"
#include <cassert>
#include <QThread>

class Thread : public QThread
{
public:
    static void usleep(unsigned long usecs)
    {
        QThread::usleep(usecs);
    }
};
/*!
default constructor
*/
QxtJob::QxtJob()
{
    QXT_INIT_PRIVATE(QxtJob);
    qxt_d().running.set(false);
    connect(&qxt_d(), SIGNAL(done()), this, SIGNAL(done()));
}
/*!
execute the Job on \a onthread
*/
void QxtJob::exec(QThread * onthread)
{
    qxt_d().moveToThread(onthread);
    connect(this, SIGNAL(subseed()), &qxt_d(), SLOT(inwrap_d()), Qt::QueuedConnection);

    qxt_d().running.set(true);
    emit(subseed());
}
/*!
\warning The destructor joins. Means it blocks until the job is finished
*/
QxtJob::~QxtJob()
{
    join();
}
/*!
block until the Job finished
Note that the current thread will be blocked.
If you use this, you better be damn sure you actually want a thread.
Maybe you actualy want to use QxtSignalWaiter.
*/
void QxtJob::join()
{
    while (qxt_d().running.get() == true)
    {
        /*!
        oh yeah that sucks ass,
        but since any kind of waitcondition will just fail due to undeterminnism,
        we have no chance then polling.
        And no, a mutex won't work either.
        using join for anything else then testcases sounds kindof retarded anyway.
        */
        Thread::usleep(1000);
    }

}
void QxtJobPrivate::inwrap_d()
{
    synca.wakeAll();
    qxt_p().run();
    running.set(false);
    emit(done());
}









