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

#ifndef QXTTCPCONNECTIONMANAGER_H
#define QXTTCPCONNECTIONMANAGER_H

#include <qxtabstractconnectionmanager.h>
#include <QObject>
#include <QNetworkProxy>
QT_FORWARD_DECLARE_CLASS(QIODevice)

class QxtTcpConnectionManagerPrivate;
class QXT_NETWORK_EXPORT QxtTcpConnectionManager : public QxtAbstractConnectionManager
{
    Q_OBJECT
public:
    QxtTcpConnectionManager(QObject* parent);

    bool listen(QHostAddress iface = QHostAddress::Any, int port = 80);
    void stopListening();
    bool isAcceptingConnections() const;

    void setProxy(const QNetworkProxy& proxy);
    QNetworkProxy proxy() const;

protected:
    virtual QIODevice* incomingConnection(int socketDescriptor);
    virtual void removeConnection(QIODevice* device, quint64 clientID);

private:
    QXT_DECLARE_PRIVATE(QxtTcpConnectionManager)
};

#endif
