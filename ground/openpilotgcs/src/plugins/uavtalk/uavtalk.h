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

#include <QIODevice>
#include <QMutex>
#include <QMutexLocker>
#include <QSemaphore>
#include "uavobjectmanager.h"
#include "uavtalk_global.h"

class UAVTALK_EXPORT UAVTalk: public QObject
{
    Q_OBJECT

public:
    typedef struct {
        quint32 txBytes;
        quint32 rxBytes;
        quint32 txObjectBytes;
        quint32 rxObjectBytes;
        quint32 rxObjects;
        quint32 txObjects;
        quint32 txErrors;
        quint32 rxErrors;
    } ComStats;

    UAVTalk(QIODevice* iodev, UAVObjectManager* objMngr);
    ~UAVTalk();
    bool sendObject(UAVObject* obj, bool acked, bool allInstances);
    bool sendObjectRequest(UAVObject* obj, bool allInstances);
    void cancelTransaction();
    ComStats getStats();
    void resetStats();

signals:
    void transactionCompleted(UAVObject* obj, bool success);

private slots:
    void processInputStream(void);

private:
    // Constants
    static const int TYPE_MASK = 0xF8;
    static const int TYPE_VER = 0x20;
    static const int TYPE_OBJ = (TYPE_VER | 0x00);
    static const int TYPE_OBJ_REQ = (TYPE_VER | 0x01);
    static const int TYPE_OBJ_ACK = (TYPE_VER | 0x02);
    static const int TYPE_ACK = (TYPE_VER | 0x03);
    static const int TYPE_NACK = (TYPE_VER | 0x04);

    static const int MIN_HEADER_LENGTH = 8; // sync(1), type (1), size(2), object ID(4)
    static const int MAX_HEADER_LENGTH = 10; // sync(1), type (1), size(2), object ID (4), instance ID(2, not used in single objects)

    static const int CHECKSUM_LENGTH = 1;

    static const int MAX_PAYLOAD_LENGTH = 256;

    static const int MAX_PACKET_LENGTH = (MAX_HEADER_LENGTH + MAX_PAYLOAD_LENGTH + CHECKSUM_LENGTH);

    static const quint16 ALL_INSTANCES = 0xFFFF;
    static const quint16 OBJID_NOTFOUND = 0x0000;

    static const int TX_BUFFER_SIZE = 2*1024;
    static const quint8 crc_table[256];

    // Types
    typedef enum {STATE_SYNC, STATE_TYPE, STATE_SIZE, STATE_OBJID, STATE_INSTID, STATE_DATA, STATE_CS} RxStateType;

    // Variables
    QIODevice* io;
    UAVObjectManager* objMngr;
    QMutex* mutex;
    UAVObject* respObj;
    bool respAllInstances;
    quint8 rxBuffer[MAX_PACKET_LENGTH];
    quint8 txBuffer[MAX_PACKET_LENGTH];
    // Variables used by the receive state machine
    quint8 rxTmpBuffer[4];
    quint8 rxType;
    quint32 rxObjId;
    quint16 rxInstId;
    quint16 rxLength;
    quint16 rxPacketLength;

    quint8 rxCSPacket, rxCS;
    qint32 rxCount;
    qint32 packetSize;
    RxStateType rxState;
    ComStats stats;

    // Methods
    bool objectTransaction(UAVObject* obj, quint8 type, bool allInstances);
    bool processInputByte(quint8 rxbyte);
    bool receiveObject(quint8 type, quint32 objId, quint16 instId, quint8* data, qint32 length);
    UAVObject* updateObject(quint32 objId, quint16 instId, quint8* data);
    void updateAck(UAVObject* obj);
    void updateNack(UAVObject* obj);
    bool transmitNack(quint32 objId);
    bool transmitObject(UAVObject* obj, quint8 type, bool allInstances);
    bool transmitSingleObject(UAVObject* obj, quint8 type, bool allInstances);
    quint8 updateCRC(quint8 crc, const quint8 data);
    quint8 updateCRC(quint8 crc, const quint8* data, qint32 length);

};

#endif // UAVTALK_H
