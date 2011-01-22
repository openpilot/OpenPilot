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

#ifndef QXTABSTRACTCONNECTIONMANAGER_H
#define QXTABSTRACTCONNECTIONMANAGER_H

#include <QObject>
#include <qxtglobal.h>

QT_FORWARD_DECLARE_CLASS(QIODevice)

class QxtAbstractConnectionManagerPrivate;
class QXT_CORE_EXPORT QxtAbstractConnectionManager : public QObject
{
    Q_OBJECT
public:
    QxtAbstractConnectionManager(QObject* parent);
    virtual ~QxtAbstractConnectionManager();

    int clientCount() const;
    QList<quint64> clients() const;
    QIODevice* client(quint64 clientID) const;

    /*!
     * Returns true if the connection manager is currently accepting connections.
     * Returns false otherwise.
     */
    virtual bool isAcceptingConnections() const = 0;

Q_SIGNALS:
    /*!
     * Indicates that the specified device, with the specified client ID, is ready for use.
     */
    void newConnection(QIODevice* device, quint64 clientID);

    /*!
     * Indicates that the device with the specified client ID has been disconnected.
     */
    void disconnected(QIODevice* device, quint64 clientID);

public Q_SLOTS:
    void disconnect(quint64 clientID);

protected:
    void addConnection(QIODevice* device, quint64 clientID);

    /*!
     * Stops managing a connection. This function is invoked by disconnect().
     * Implementations should perform whatever steps are necessary to close the connection
     * and clean up any internal data structures, including deleting the QIODevice object.
     */
    virtual void removeConnection(QIODevice* device, quint64 clientID) = 0;

private:
    QXT_DECLARE_PRIVATE(QxtAbstractConnectionManager)
};

#endif
