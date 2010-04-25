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
#include "qxtstdio_p.h"
#include <cstdio>

/*!
\class QxtStdio

\inmodule QxtCore

\brief The QxtStdio class provides QIODevice and QxtPipe support for stdin and stdout

including readyRead() signal.
perfect as a counter part for QProcess or debug output into a QxtPipe chain


\sa QIODevice
\sa QxtPipe


\bold {Note:} when using this class, the buffers for stdin/stdout will be disabled, and NOT reenabled on destruction

*/




/*!
    Constructs a new QxtStdio with \a parent.
 */
QxtStdio::QxtStdio(QObject * parent): QxtPipe(parent)
{
    QXT_INIT_PRIVATE(QxtStdio);

    setvbuf(stdin , NULL , _IONBF , 0);
    setvbuf(stdout , NULL , _IONBF , 0);

    setOpenMode(QIODevice::ReadWrite);
    qxt_d().notify = new QSocketNotifier(

#ifdef Q_CC_MSVC
        _fileno(stdin)
#else
        fileno(stdin)
#endif

        , QSocketNotifier::Read, this);
    QObject::connect(qxt_d().notify, SIGNAL(activated(int)), &qxt_d(), SLOT(activated(int)));
}

/*!
    \reimp
 */
qint64 QxtStdio::writeData(const char * data, qint64 maxSize)
{
    qint64 i = 0;
    for (;i < maxSize;i++)
    {
        char c = *data++;
        putchar(c);
    }
//  emit(bytesWritten (i)); ///FIXME: according to the docs this may not be recoursive. how do i prevent that?
    return i;
}



void QxtStdioPrivate::activated(int)
{
    char c = getchar();
    if (c == EOF)
    {
#if QT_VERSION >= 0x040400
        emit qxt_p().readChannelFinished();
#endif
        hadeof = true;
        return;
    }
    QByteArray b(1, c);
    qxt_p().enqueData(b);
    qxt_p().sendData(b);
}

/*!
    Receive \a data.
 */
void   QxtStdio::receiveData(QByteArray data, const QxtPipe *)
{
    writeData(data.data(), data.size());
}

/*!
    \reimp
 */
bool QxtStdio::waitForReadyRead(int)
{
    if (qxt_d().hadeof)
        return false;


    char c = getchar();
    if (c == EOF)
    {
#if QT_VERSION >= 0x040400
        emit readChannelFinished();
#endif
        qxt_d().hadeof = true;
        return false;
    }
    QByteArray b(1, c);
    enqueData(b);
    sendData(b);
    return true;
}

/*!Blocks until EOF is received.*/
void QxtStdio::waitForEOF()
{
    if (qxt_d().hadeof)
        return;

    forever
    {
        char c = getchar();
        if (c == EOF)
        {
#if QT_VERSION >= 0x040400
            emit readChannelFinished();
#endif
            qxt_d().hadeof = true;
            return;
        }
        QByteArray b(1, c);
        enqueData(b);
        sendData(b);
    }
}


