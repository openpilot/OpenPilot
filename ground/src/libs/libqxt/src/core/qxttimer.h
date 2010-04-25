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
#ifndef QXTTIMER_H
#define QXTTIMER_H

#include <QTimer>
#include <QVariant>
#include <qxtglobal.h>

class QXT_CORE_EXPORT QxtTimer : public QTimer
{
public:
    explicit QxtTimer(QObject* parent = 0);
    virtual ~QxtTimer();

    using QTimer::singleShot;
    static void singleShot(int msec, QObject* receiver, const char* member, const QVariant& arg0,
                           const QVariant& arg1 = QVariant(), const QVariant& arg2 = QVariant(), 
                           const QVariant& arg3 = QVariant(), const QVariant& arg4 = QVariant(), 
                           const QVariant& arg5 = QVariant(), const QVariant& arg6 = QVariant(), 
                           const QVariant& arg7 = QVariant(), const QVariant& arg8 = QVariant(), 
                           const QVariant& arg9 = QVariant());
};

#endif // QXTTIMER_H
