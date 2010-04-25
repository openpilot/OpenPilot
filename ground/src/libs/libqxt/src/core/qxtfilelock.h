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

#ifndef QXTFILELOCK_H
#define QXTFILELOCK_H

#include <QFile>
#include <QObject>
#include "qxtglobal.h"

#ifdef Q_OS_WIN
#include <windows.h>
typedef long off_t;
#else
#include <sys/types.h>
#endif

class QxtFileLockPrivate;

class QXT_CORE_EXPORT QxtFileLock : public QObject
{
    Q_OBJECT

public:

    enum Mode
    {
        ReadLockWait,
        ReadLock,
        WriteLockWait,
        WriteLock
    };

    QxtFileLock(QFile *file, const off_t offset, const off_t length, const QxtFileLock::Mode mode = WriteLockWait);
    ~QxtFileLock();

    off_t offset() const;
    off_t length() const;
    bool isActive() const;
    QFile *file() const;
    QxtFileLock::Mode mode() const;

public Q_SLOTS:
    bool lock();
    bool unlock();

private:
    QxtFileLock(const QxtFileLock &other);
    QXT_DECLARE_PRIVATE(QxtFileLock)
};

#endif // QXTFILELOCK_H
