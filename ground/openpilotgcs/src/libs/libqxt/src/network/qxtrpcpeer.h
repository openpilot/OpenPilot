/****************************************************************************
 **
 ** Copyright (C) Qxt Foundation. Some rights reserved.
 **
 ** This file is part of the QxtNetwork module of the Qxt library.
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

#ifndef QXTRPCPEER_H
#define QXTRPCPEER_H

#include <qxtrpcservice.h>
#include <QString>
#include <QHostAddress>
#include <QAbstractSocket>

class QxtRPCPeerPrivate;
class QXT_NETWORK_EXPORT QxtRPCPeer : public QxtRPCService
{
    Q_OBJECT
public:
    QxtRPCPeer(QObject* parent = 0);

    void connect(QHostAddress addr, int port = 80);
    void connect(QString addr, int port = 80);

    bool listen(QHostAddress iface = QHostAddress::Any, int port = 80);
    void stopListening();

Q_SIGNALS:
    /*!
     * This signal is emitted after a successful connection to a server.
     */
    void connectedToServer();

    /*!
     * This signal is emitted after disconnecting from a server.
     */
    void disconnectedFromServer();

    /*!
     * This signal is emitted if an error occurs on the socket connected to the server.
     */
    void serverError(QAbstractSocket::SocketError);

private:
    QXT_DECLARE_PRIVATE(QxtRPCPeer)
};
#endif
