#ifndef UAVTALK_H
#define UAVTALK_H

#include <QIODevice>
#include <QMutex>
#include <QMutexLocker>
#include <QSemaphore>
#include "uavobjects\uavobjectmanager.h"

class UAVTalk: public QObject
{
    Q_OBJECT

public:
    UAVTalk(QIODevice* iodev, UAVObjectManager* objMngr);

    qint32 sendObject(UAVObject* obj, bool acked, qint32 timeoutMs);
    qint32 sendObjectRequest(UAVObject* obj, qint32 timeout, bool allInstances);

signals:
    void transactionCompleted(UAVObject* obj);

private slots:
    void processInputStream(void);

private:
    // Constants
    static const int TYPE_MASK = 0xFC;
    static const int TYPE_VER = 0x10;
    static const int TYPE_OBJ = (TYPE_VER | 0x00);
    static const int TYPE_OBJ_REQ = (TYPE_VER | 0x01);
    static const int TYPE_OBJ_ACK = (TYPE_VER | 0x02);
    static const int TYPE_ACK = (TYPE_VER | 0x03);
    static const int HEADER_LENGTH = 7; // type (1), object ID (4), instance ID (2, not used in single objects)
    static const int CHECKSUM_LENGTH = 2;
    static const int MAX_PAYLOAD_LENGTH = 256;
    static const int MAX_PACKET_LENGTH = (HEADER_LENGTH+MAX_PAYLOAD_LENGTH+CHECKSUM_LENGTH);
    static const quint16 ALL_INSTANCES = 0xFFFF;

    typedef enum {STATE_SYNC, STATE_OBJID, STATE_INSTID, STATE_DATA, STATE_CS} RxStateType;

    // Variables
    QIODevice* io;
    UAVObjectManager* objMngr;
    QMutex* mutex;
    QSemaphore* respSema;
    UAVObject* respObj;
    bool respAllInstances;
    quint8 rxBuffer[MAX_PACKET_LENGTH];
    quint8 txBuffer[MAX_PACKET_LENGTH];
    // Variables used by the receive state machine
    quint8 rxTmpBuffer[4];
    quint8 rxType;
    quint32 rxObjId;
    quint16 rxInstId;
    quint8 rxLength;
    quint16 rxCSPacket, rxCS;
    qint32 rxCount;
    RxStateType rxState;

    // Methods
    bool objectTransaction(UAVObject* obj, quint8 type, qint32 timeoutMs, bool allInstances);
    bool processInputByte(quint8 rxbyte);
    bool receiveObject(quint8 type, quint32 objId, quint16 instId, quint8* data, qint32 length);
    UAVObject* updateObject(quint32 objId, quint16 instId, quint8* data);
    void updateAck(UAVObject* obj);
    bool transmitObject(UAVObject* obj, quint8 type, bool allInstances);
    bool transmitSingleObject(UAVObject* obj, quint8 type, bool allInstances);
    quint16 updateChecksum(quint16 cs, quint8* data, qint32 length);

};

#endif // UAVTALK_H
