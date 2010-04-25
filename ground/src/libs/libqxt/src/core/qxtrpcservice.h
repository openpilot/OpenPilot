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

#ifndef QXTRPCSERVICE_H
#define QXTRPCSERVICE_H

#include <QObject>
#include <QList>
#include <QVariant>
#include <QPair>
#include <QString>
#include <qxtglobal.h>

QT_FORWARD_DECLARE_CLASS(QIODevice)
class QxtAbstractConnectionManager;
class QxtAbstractSignalSerializer;

class QxtRPCServicePrivate;
class QXT_CORE_EXPORT QxtRPCService : public QObject
{
Q_OBJECT
public:
    QxtRPCService(QObject* parent = 0);
    explicit QxtRPCService(QIODevice* device, QObject* parent = 0);
    virtual ~QxtRPCService();

    bool isServer() const;
    bool isClient() const;

    QList<quint64> clients() const;
    QIODevice* device() const;
    void setDevice(QIODevice* dev);
    QIODevice* takeDevice();

    QxtAbstractSignalSerializer* serializer() const;
    void setSerializer(QxtAbstractSignalSerializer* serializer);

    QxtAbstractConnectionManager* connectionManager() const;
    void setConnectionManager(QxtAbstractConnectionManager* manager);

    bool attachSignal(QObject* sender, const char* signal, const QString& rpcFunction = QString());
    bool attachSlot(const QString& rpcFunction, QObject* recv, const char* slot,
            Qt::ConnectionType type = Qt::AutoConnection);
    void detachSignals(QObject* obj);
    void detachSlots(QObject* obj);
    void detachObject(QObject* obj);

public Q_SLOTS:
    void disconnectClient(quint64 id);
    void disconnectServer();
    void disconnectAll();

    void call(QString fn, const QVariant& p1 = QVariant(), const QVariant& p2 = QVariant(),
              const QVariant& p3 = QVariant(), const QVariant& p4 = QVariant(), const QVariant& p5 = QVariant(),
              const QVariant& p6 = QVariant(), const QVariant& p7 = QVariant(), const QVariant& p8 = QVariant());
    void call(QList<quint64> ids, QString fn, const QVariant& p1 = QVariant(), const QVariant& p2 = QVariant(),
              const QVariant& p3 = QVariant(), const QVariant& p4 = QVariant(), const QVariant& p5 = QVariant(),
              const QVariant& p6 = QVariant(), const QVariant& p7 = QVariant(), const QVariant& p8 = QVariant());
    void call(quint64 id, QString fn, const QVariant& p1 = QVariant(), const QVariant& p2 = QVariant(),
              const QVariant& p3 = QVariant(), const QVariant& p4 = QVariant(), const QVariant& p5 = QVariant(),
              const QVariant& p6 = QVariant(), const QVariant& p7 = QVariant(), const QVariant& p8 = QVariant());
    void callExcept(quint64 id, QString fn, const QVariant& p1 = QVariant(), const QVariant& p2 = QVariant(),
                    const QVariant& p3 = QVariant(), const QVariant& p4 = QVariant(), const QVariant& p5 = QVariant(),
                    const QVariant& p6 = QVariant(), const QVariant& p7 = QVariant(), const QVariant& p8 = QVariant());

    void detachSender();

Q_SIGNALS:
    /*!
     * This signal is emitted after a successful connection from a client.
     *
     * The given ID is used for disconnectClient(), call(), and related functions.
     */
    void clientConnected(quint64 id);

    /*!
     * This signal is emitted when a client disconnects. The given ID is no longer valid.
     */
    void clientDisconnected(quint64 id);

private:
    QXT_DECLARE_PRIVATE(QxtRPCService)
};
#endif
