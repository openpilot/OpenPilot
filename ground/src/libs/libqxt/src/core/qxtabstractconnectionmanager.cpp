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

#include "qxtabstractconnectionmanager.h"
#include <QHash>
#include <QtDebug>

/*!
 * \class QxtAbstractConnectionManager
 * \inmodule QxtCore
 * \brief The QxtAbstractConnectionManager class is an interface for classes that maintain connection pools
 *
 * QxtAbstractConnectionManager is a standardized interface for accepting and tracking
 * incoming connections.
 *
 * Each incoming connection is assigned an arbitrary, opaque client ID number. This
 * number can be used to retrieve the QIODevice associated with it. A list of IDs
 * for all current connections can be retrieved with the clients() function.
 *
 * As an abstract class, QxtAbstractConnectionManager cannot be used on its own.
 * Subclasses must implement isAcceptingConnections() to indicate the current listening
 * status of the connection manager and removeConnection() to perform necessary clean-up
 * of an established connection before disconnecting. Finally, a subclass must invoke
 * addConnection() after a new incoming connection has been established and prepared.
 *
 * \sa QxtTcpConnectionManager
 */

class QxtAbstractConnectionManagerPrivate : public QxtPrivate<QxtAbstractConnectionManager>
{
public:
    QHash<quint64, QIODevice*> clients;

    QXT_DECLARE_PUBLIC(QxtAbstractConnectionManager)
};

/*!
 * Constructs a QxtAbstractConnectionManager object with the specified \a parent.
 */
QxtAbstractConnectionManager::QxtAbstractConnectionManager(QObject* parent) : QObject(parent)
{
    QXT_INIT_PRIVATE(QxtAbstractConnectionManager);
}

/*!
 * Destructs the connection manager.
 */
QxtAbstractConnectionManager::~QxtAbstractConnectionManager()
{
    // virtual for subclasses, no need for implementation
}

/*!
 * Returns the number of connected clients.
 */
int QxtAbstractConnectionManager::clientCount() const
{
    return qxt_d().clients.count();
}

/*!
 * Returns a list of client IDs for all connected clients.
 */
QList<quint64> QxtAbstractConnectionManager::clients() const
{
    return qxt_d().clients.keys();
}

/*!
 * Returns the QIODevice for the requested \a clientID.
 */
QIODevice* QxtAbstractConnectionManager::client(quint64 clientID) const
{
    return qxt_d().clients.value(clientID, NULL);
}

/*!
 * Disconnects the connection with the specified \a clientID.
 *
 * Implementations should invoke this function when the connection should no longer be
 * used; for instance, when the QIODevice object is destroyed or the connection has been
 * closed. Code that uses the connection manager service should invoke this function to
 * close a connection on demand.
 */
void QxtAbstractConnectionManager::disconnect(quint64 clientID)
{
    QIODevice* device = qxt_d().clients.value(clientID, 0);
    if (!device)
    {
        qWarning() << "QxtAbstractConnectionManager::disconnect: client ID not in use";
        return;
    }
    qxt_d().clients.remove(clientID);
    emit disconnected(device, clientID);
    removeConnection(device, clientID);
}

/*!
 * Begins managing a connection. Each \a device must have a unique, non-zero \a clientID, but
 * beyond this constraint implementations may use any convenient value. Implementations
 * should invoke this function after establishing a usable connection and performing any
 * desired initial negotiation.
 *
 * This function will emit the newConnection signal after the internal data structures have
 * been suitably updated.
 */
void QxtAbstractConnectionManager::addConnection(QIODevice* device, quint64 clientID)
{
    qxt_d().clients[clientID] = device;
    emit newConnection(device, clientID);
}
