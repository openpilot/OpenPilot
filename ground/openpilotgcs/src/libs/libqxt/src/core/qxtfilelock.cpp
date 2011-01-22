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
#include "qxtfilelock.h"
#include "qxtfilelock_p.h"
/*!
 * \class QxtFileLock
 * \inmodule QxtCore
 * \brief The QxtFileLock class provides a crossplattform way to lock a QFile.
 *
 * It supports the range locking of a file. The File will take parentship of the lock.<br>
 * The lock gets cleaned up with the QFile, and it is released when the QFile is closed.<br>
 *
 * Example usage:
 * \code
 * off_t lockstart = 0x10;
 * off_t locklength = 30
 *
 * QFile file("test.lock");
 *
 * //the lock gets deleted when file is cleaned up
 * QxtFileLock * writeLock = new QxtFileLock(&file,lockstart,locklength,QxtFileLock::WriteLock);
 * if(file.open(QIODevice::ReadWrite))
 * {
 *     if(writeLock->lock())
 *     {
 *          // some write operations
 *         writeLock->unlock();
 *     }
 *      else
 *          //lock failed
 * }
 * \endcode
 * \bold {Note:} QxtFileLock behaves different than normal unix locks on *nix. A thread can writelock the region of a file only ONCE if it uses two  different handles.
 *           A different thread can not writelock a region that is owned by a other thread even if it is the SAME process.
 * \bold {Note:} On *nix this class uses fctnl to lock the file. This may not be compatible to other locking functions like flock and lockf
 * \bold {Note:} Please do not mix QxtFileLock and native file lock calls on the same QFile. The behaviour is undefined
 * \bold {Note:} QxtFileLock lives in the same thread as the passed QFile
 * \warning due to a refactoring issues of QFile this class will not work with Qt from version 4.3.0 to 4.3.2
*/

/*!
 * \fn bool QxtFileLock::lock()
 * \brief Locks the file
 * Returns \c true if succeeds, \c false otherwise.
 */

/*!
 * \fn bool QxtFileLock::unlock()
 * \brief Unlocks the file.
 * Returns \c true if succeeds, \c false otherwise.
 */

/*!
 * \enum QxtFileLock::Mode
 * \brief The mode of the lock
 *
 * \value ReadLock A non blocking read lock
 * \value WriteLock A non blocking write lock
 * \value ReadLockWait A  blocking read lock. The lock() function will block until the lock is created.
 * \value WriteLockWait A blocking write lock. The lock() function will block until the lock is created.
 */

QxtFileLockPrivate::QxtFileLockPrivate()  : offset(0), length(0), mode(QxtFileLock::WriteLockWait), isLocked(false)
{
}

/*!
 * Contructs a new QxtFileLock. The lock is not activated.
 * \a file the file that should be locked
 * \a offset the offset where the lock starts
 * \a length the length of the lock
 * \a mode the lockmode
 */
QxtFileLock::QxtFileLock(QFile *file, const off_t offset, const off_t length, const QxtFileLock::Mode mode) : QObject(file)
{
    QXT_INIT_PRIVATE(QxtFileLock);
    connect(file, SIGNAL(aboutToClose()), this, SLOT(unlock()));
    qxt_d().offset = offset;
    qxt_d().length = length;
    qxt_d().mode = mode;
}

/*!
 * Destructs the file lock.
 */
QxtFileLock::~QxtFileLock()
{
    unlock();
}

/*!
 * Returns the offset of the lock
 */
off_t QxtFileLock::offset() const
{
    return qxt_d().offset;
}

/*!
 * Returns \c true if the lock is active otherwise it returns \c false
 */
bool QxtFileLock::isActive() const
{
    return qxt_d().isLocked;
}

/*!
 * Returns the length of the lock
 */
off_t QxtFileLock::length() const
{
    return qxt_d().length;
}

/*!
 * Returns the file the lock is created on.
 */
QFile * QxtFileLock::file() const
{
    return qobject_cast<QFile *>(parent());
}

/*!
 * Returns the mode of the lock
 */
QxtFileLock::Mode QxtFileLock::mode() const
{
    return qxt_d().mode;
}
