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

#include "qxttcpconnectionmanager.h"
#include "qxttcpconnectionmanager_p.h"
#include <QTcpSocket>
#include <QtDebug>

/*!
 * \class QxtTcpConnectionManager
 * \inmodule QxtNetwork
 * \brief The QxtTcpConnectionManager class accepts TCP connections and maintains a connection pool
 *
 * QxtTcpConnectionManager is a standardized interface for accepting and tracking
 * incoming TCP connections.
 *
 * Each incoming connection is assigned an arbitrary, opaque client ID number. This
 * number can be used to retrieve the QTcpSocket associated with it. A list of IDs
 * for all current connections can be retrieved with the clients() function.
 *
 * Like QTcpServer, QxtTcpConnectionManager can listen for incoming connections on
 * a specified interface and port, and like QTcpServer you may override the
 * incomingConnection() function to change the handling of new connections. This
 * is, for instance, where you would create a QSslSocket to encrypt communications.
 *
 * \sa QTcpServer
 */

/*!
 * Constructs a new QxtTcpConnectionManager object with the specified \a parent.
 */
QxtTcpConnectionManager::QxtTcpConnectionManager(QObject* parent) : QxtAbstractConnectionManager(parent)
{
    QXT_INIT_PRIVATE(QxtTcpConnectionManager);
}

QxtTcpConnectionManagerPrivate::QxtTcpConnectionManagerPrivate() : QTcpServer(0)
{
    QObject::connect(&mapper, SIGNAL(mapped(QObject*)), this, SLOT(socketDisconnected(QObject*)));
}

void QxtTcpConnectionManagerPrivate::incomingConnection(int socketDescriptor)
{
    QIODevice* device = qxt_p().incomingConnection(socketDescriptor);
    if (device)
    {
        qxt_p().addConnection(device, (quint64)static_cast<QObject*>(device));
        mapper.setMapping(device, device);
        QObject::connect(device, SIGNAL(destroyed()), &mapper, SLOT(map()));
        QTcpSocket* sock = qobject_cast<QTcpSocket*>(device);
        if (sock)
        {
            QObject::connect(sock, SIGNAL(error(QAbstractSocket::SocketError)), &mapper, SLOT(map()));
            QObject::connect(sock, SIGNAL(disconnected()), &mapper, SLOT(map()));
        }
    }
}

/*!
 * Listens on the specified interface \a iface on the specified \a port for connections.
 * If \a iface is QHostAddress::Any, listens on all interfaces.
 *
 * Returns \c true on success; otherwise returns \c false.
 */
bool QxtTcpConnectionManager::listen(QHostAddress iface, int port)
{
    return qxt_d().listen(iface, port);
}

/*!
 * Stops listening for connections. Any connections still open will remain connected.
 */
void QxtTcpConnectionManager::stopListening()
{
    if (!qxt_d().isListening())
    {
        qWarning() << "QxtTcpConnectionManager: Not listening";
        return;
    }
    qxt_d().close();
}

/*!
 * \reimp
 */
bool QxtTcpConnectionManager::isAcceptingConnections() const
{
    return qxt_d().isListening();
}

/*!
 * This function is called when a new TCP connection becomes available. The parameter
 * is the native \a socketDescriptor for the connection, suitable for use in
 * QTcpSocket::setSocketDescriptor.
 *
 * The default implementation returns a new QTcpSocket with the specified descriptor.
 * Subclasses may return QTcpSocket subclasses, such as QSslSocket.
 */
QIODevice* QxtTcpConnectionManager::incomingConnection(int socketDescriptor)
{
    QTcpSocket* device = new QTcpSocket(this);
    device->setSocketDescriptor(socketDescriptor);
    return device;
}

/*!
 * \reimp
 */
void QxtTcpConnectionManager::removeConnection(QIODevice* device, quint64 clientID)
{
    Q_UNUSED(clientID);
    if (device)
    {
        QAbstractSocket* sock = qobject_cast<QAbstractSocket*>(device);
        if (sock) sock->disconnectFromHost();
        device->close();
        device->deleteLater();
    }
}

/*!
 * Sets an explicit network \a proxy for the connection manager.
 *
 * \sa QTcpServer::setProxy()
 */
void QxtTcpConnectionManager::setProxy(const QNetworkProxy& proxy)
{
    qxt_d().setProxy(proxy);
}

/*!
 * Returns the proxy in use for the connection manager.
 *
 * \sa QTcpServer::proxy()
 */
QNetworkProxy QxtTcpConnectionManager::proxy() const
{
    return qxt_d().proxy();
}

void QxtTcpConnectionManagerPrivate::socketDisconnected(QObject* client)
{
    QTcpSocket* sock = qobject_cast<QTcpSocket*>(client);
    if (sock)
    {
        QObject::disconnect(sock, SIGNAL(error(QAbstractSocket::SocketError)), &mapper, SLOT(map()));
        QObject::disconnect(sock, SIGNAL(disconnected()), &mapper, SLOT(map()));
    }
    qxt_p().disconnect((quint64)(client));
}
