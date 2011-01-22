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
#include "qxtstdstreambufdevice.h"

/*!
\class QxtStdStreambufDevice

\inmodule QxtCore

\brief The QxtStdStreambufDevice class provides QIODevice support for std::streambuf

\warning does NOT emit the readyRead() signal

\sa QIODevice

*/

/*!
\brief creates a QxtStdStreambufDevice using a single stream buffer as in and output
*/
QxtStdStreambufDevice::QxtStdStreambufDevice(std::streambuf * b, QObject * parent): QIODevice(parent), buff(b)
{
    setOpenMode(QIODevice::ReadWrite);  //we don't know the real state
    buff_w = 0;
}

/*!
\brief creates a QxtStdStreambufDevice using \a r to read and \a w to write
*/
QxtStdStreambufDevice::QxtStdStreambufDevice(std::streambuf * r, std::streambuf * w, QObject * parent): QIODevice(parent), buff(r), buff_w(w)
{
    setOpenMode(QIODevice::ReadWrite);
}

/*!
\reimp
*/
bool QxtStdStreambufDevice::isSequential() const
{
    return true;//for now
}

/*!
\reimp
*/
qint64 QxtStdStreambufDevice::bytesAvailable() const
{
    return buff->in_avail();
}

/*!
\reimp
*/
qint64 QxtStdStreambufDevice::readData(char * data, qint64 maxSize)
{
    return buff->sgetn(data, maxSize);
}

/*!
\reimp
*/
qint64 QxtStdStreambufDevice::writeData(const char * data, qint64 maxSize)
{
    if (buff_w)
        return buff_w->sputn(data, maxSize);
    return buff->sputn(data, maxSize);
}
