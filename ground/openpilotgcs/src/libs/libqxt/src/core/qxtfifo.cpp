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
\class QxtFifo

\inmodule QxtCore

\brief The QxtFifo class provides a simple loopback QIODevice.

read and write to the same object
emits a readyRead Signal.
useful for loopback tests where QBuffer does not work.

\code
QxtFifo fifo;
 QTextStream (&fifo)<<QString("foo");
 QString a;
 QTextStream(&fifo)>>a;
 qDebug()<<a;
\endcode

*/



#include "qxtfifo.h"
#include <string.h>
#include <limits.h>
#include <QDebug>
#include <QQueue>

#include <qatomic.h>

#if QT_VERSION >= 0x040400
# include <qbasicatomic.h>
# define QXT_EXCHANGE fetchAndStoreOrdered
# define QXT_ADD fetchAndAddOrdered
#else
  typedef QBasicAtomic QBasicAtomicInt;
# define QXT_EXCHANGE exchange
# define QXT_ADD fetchAndAdd
#endif

struct QxtFifoNode {
    QxtFifoNode(const char* data, int size) : content(data, size) {
        next = NULL;
    }
   
    QByteArray content;
    QBasicAtomicPointer<QxtFifoNode> next;
};

class QxtFifoPrivate : public QxtPrivate<QxtFifo> {
public:
    QXT_DECLARE_PUBLIC(QxtFifo)
    QxtFifoPrivate() {
        head = tail = new QxtFifoNode(NULL, 0);
        available = 0;
    }

    QBasicAtomicPointer<QxtFifoNode> head, tail;
    QBasicAtomicInt available;
};

/*!
Constructs a new QxtFifo with \a parent.
*/
QxtFifo::QxtFifo(QObject *parent) : QIODevice(parent)
{
    QXT_INIT_PRIVATE(QxtFifo);
    setOpenMode(QIODevice::ReadWrite);
}

/*!
\reimp
*/
qint64 QxtFifo::readData ( char * data, qint64 maxSize )
{
    int bytes = qxt_d().available, step;
    if(!bytes) return 0;
    if(bytes > maxSize) bytes = maxSize;
    int written = bytes;
    char* writePos = data;
    QxtFifoNode* node;
    while(bytes > 0) {
        node = qxt_d().head;
        step = node->content.size();
        if(step >= bytes) {
            int rem = step - bytes;
            memcpy(writePos, node->content.constData(), bytes);
            step = bytes;
            node->content = node->content.right(rem);
        } else {
            memcpy(writePos, node->content.constData(), step);
            qxt_d().head.QXT_EXCHANGE(node->next);
            delete node;
            node = qxt_d().head;
        }
        writePos += step;
        bytes -= step;
    }
    qxt_d().available.QXT_ADD(-written);
    return written;
}

/*!
\reimp
*/
qint64 QxtFifo::writeData ( const char * data, qint64 maxSize )
{
    if(maxSize > 0) {
        if(maxSize > INT_MAX) maxSize = INT_MAX; // qint64 could easily exceed QAtomicInt, so let's play it safe
        QxtFifoNode* newData = new QxtFifoNode(data, maxSize);
        qxt_d().tail->next.QXT_EXCHANGE(newData);
        qxt_d().tail.QXT_EXCHANGE(newData);
        qxt_d().available.QXT_ADD(maxSize);
        QMetaObject::invokeMethod(this, "bytesWritten", Qt::QueuedConnection, Q_ARG(qint64, maxSize));
        QMetaObject::invokeMethod(this, "readyRead", Qt::QueuedConnection);
    }
    return maxSize;
}

/*!
\reimp
*/
bool QxtFifo::isSequential () const
{
    return true;
}

/*!
\reimp
*/
qint64 QxtFifo::bytesAvailable () const
{
    return qxt_d().available;
}

/*!
*/
void QxtFifo::clear()
{
    qxt_d().available.QXT_EXCHANGE(0);
    qxt_d().tail.QXT_EXCHANGE(qxt_d().head);
    QxtFifoNode* node = qxt_d().head->next.QXT_EXCHANGE(NULL);
    while (node && node->next)
    {
        QxtFifoNode* next = node->next.QXT_EXCHANGE(NULL);
        delete node;
        node = next;
    }
    qxt_d().head->content = QByteArray();
}
