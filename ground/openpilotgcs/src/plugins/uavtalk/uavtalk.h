/**
 ******************************************************************************
 *
 * @file       uavtalk.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVTalkPlugin UAVTalk Plugin
 * @{
 * @brief The UAVTalk protocol plugin
 *****************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
#ifndef UAVTALK_H
#define UAVTALK_H

#include "uavobjectmanager.h"
#include "uavtalk_global.h"

#include <QtCore>
#include <QIODevice>
#include <QMutex>
#include <QMutexLocker>
#include <QMap>
#include <QThread>
#include <QtNetwork/QUdpSocket>

class UAVTALK_EXPORT UAVTalk : public QObject {
    Q_OBJECT

    friend class IODeviceReader;

public:
    static const quint16 ALL_INSTANCES = 0xFFFF;

    typedef struct {
        quint32 txBytes;
        quint32 txObjectBytes;
        quint32 txObjects;
        quint32 txErrors;

        quint32 rxBytes;
        quint32 rxObjectBytes;
        quint32 rxObjects;
        quint32 rxErrors;
        quint32 rxSyncErrors;
        quint32 rxCrcErrors;
    } ComStats;

    UAVTalk(QIODevice *iodev, UAVObjectManager *objMngr);
    ~UAVTalk();

    ComStats getStats();
    void resetStats();

    bool sendObject(UAVObject *obj, bool acked, bool allInstances);
    bool sendObjectRequest(UAVObject *obj, bool allInstances);
    void cancelTransaction(UAVObject *obj);

signals:
    void transactionCompleted(UAVObject *obj, bool success);

private slots:
    void processInputStream();
    void dummyUDPRead();

private:

    typedef struct {
        quint8  respType;
        quint32 respObjId;
        quint16 respInstId;
    } Transaction;

    // Constants
    static const int TYPE_MASK     = 0xF8;
    static const int TYPE_VER      = 0x20;
    static const int TYPE_OBJ      = (TYPE_VER | 0x00);
    static const int TYPE_OBJ_REQ  = (TYPE_VER | 0x01);
    static const int TYPE_OBJ_ACK  = (TYPE_VER | 0x02);
    static const int TYPE_ACK      = (TYPE_VER | 0x03);
    static const int TYPE_NACK     = (TYPE_VER | 0x04);

    // header : sync(1), type (1), size(2), object ID(4), instance ID(2)
    static const int HEADER_LENGTH = 10;

    static const int MAX_PAYLOAD_LENGTH = 256;

    static const int CHECKSUM_LENGTH    = 1;

    static const int MAX_PACKET_LENGTH  = (HEADER_LENGTH + MAX_PAYLOAD_LENGTH + CHECKSUM_LENGTH);

    static const int TX_BUFFER_SIZE     = 2 * 1024;

    static const quint8 crc_table[256];

    // Types
    typedef enum {
        STATE_SYNC, STATE_TYPE, STATE_SIZE, STATE_OBJID, STATE_INSTID, STATE_DATA, STATE_CS, STATE_COMPLETE, STATE_ERROR
    } RxStateType;

    // Variables
    QPointer<QIODevice> io;

    UAVObjectManager *objMngr;

    ComStats stats;

    QMutex mutex;

    QMap<quint32, QMap<quint32, Transaction *> *> transMap;

    quint8 rxBuffer[MAX_PACKET_LENGTH];

    quint8 txBuffer[MAX_PACKET_LENGTH];

    // Variables used by the receive state machine
    // state machine variables
    qint32 rxCount;
    qint32 packetSize;
    RxStateType rxState;
    // data variables
    quint8 rxTmpBuffer[4];
    quint8 rxType;
    quint32 rxObjId;
    quint16 rxInstId;
    quint16 rxLength;
    quint16 rxPacketLength;
    quint8 rxCSPacket;
    quint8 rxCS;

    bool useUDPMirror;
    QUdpSocket *udpSocketTx;
    QUdpSocket *udpSocketRx;
    QByteArray rxDataArray;

    // Methods
    bool objectTransaction(quint8 type, quint32 objId, quint16 instId, UAVObject *obj);
    bool processInputByte(quint8 rxbyte);
    bool receiveObject(quint8 type, quint32 objId, quint16 instId, quint8 *data, qint32 length);
    UAVObject *updateObject(quint32 objId, quint16 instId, quint8 *data);
    void updateAck(quint8 type, quint32 objId, quint16 instId, UAVObject *obj);
    void updateNack(quint32 objId, quint16 instId, UAVObject *obj);
    bool transmitObject(quint8 type, quint32 objId, quint16 instId, UAVObject *obj);
    bool transmitSingleObject(quint8 type, quint32 objId, quint16 instId, UAVObject *obj);

    Transaction *findTransaction(quint32 objId, quint16 instId);
    void openTransaction(quint8 type, quint32 objId, quint16 instId);
    void closeTransaction(Transaction *trans);
    void closeAllTransactions();

    const char *typeToString(quint8 type);
};

#endif // UAVTALK_H
