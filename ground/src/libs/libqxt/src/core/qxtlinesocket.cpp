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

#include "qxtlinesocket_p.h"
#include <QIODevice>

/*!
    \class QxtLineSocket

    \inmodule QxtCore

    \brief The QxtLineSocket class acts on a QIODevice as baseclass for line-based protocols
*/

/*!
    \fn QxtLineSocket::newLineReceived(const QByteArray& line)

    This signal is emitted whenever a new \a line is received.
 */

/*!
    Constructs a new QxtLineSocket with \a parent.
 */
QxtLineSocket::QxtLineSocket(QObject* parent) : QObject(parent)
{
    QXT_INIT_PRIVATE(QxtLineSocket);
}

/*!
    Constructs a new QxtLineSocket with \a socket and \a parent.
 */
QxtLineSocket::QxtLineSocket(QIODevice* socket, QObject* parent) : QObject(parent)
{
    QXT_INIT_PRIVATE(QxtLineSocket);
    setSocket(socket);
}

/*!
    Sets the \a socket.
 */
void QxtLineSocket::setSocket(QIODevice* socket)
{
    if (qxt_d().socket)
        disconnect(qxt_d().socket, SIGNAL(readyRead()), &qxt_d(), SLOT(readyRead()));
    qxt_d().socket = socket;
    if (qxt_d().socket)
        connect(qxt_d().socket, SIGNAL(readyRead()), &qxt_d(), SLOT(readyRead()));
}

/*!
    Returns the socket.
 */
QIODevice* QxtLineSocket::socket() const
{
    return qxt_d().socket;
}

/*!
    Sends a \a line.
 */
void QxtLineSocket::sendLine(const QByteArray& line)
{
    QByteArray copy(line);
    copy.replace(QByteArray("\n"), ""); //krazy:exclude=doublequote_chars
    qxt_d().socket->write(copy + '\n');
}

/*!
    This virtual function is called by QxtLineSocket whenever a \a line was received.
    Reimplement this function when creating a subclass of QxtLineSocket.

    \note The default implementation does nothing.
 */
void QxtLineSocket::newLine(const QByteArray& line)
{
    Q_UNUSED(line);
}

void QxtLineSocketPrivate::readyRead()
{
    buffer += socket->readAll();

    int i = 0;
    while ((i = buffer.indexOf('\n')) > -1)
    {
        QByteArray line = buffer.left(i);
        emit qxt_p().newLineReceived(line);
        qxt_p().newLine(line);
        buffer = buffer.mid(i + 1);
    }
}
