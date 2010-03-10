#ifndef UAVOBJECT_H
#define UAVOBJECT_H

#include <QtGlobal>
#include <QObject>
#include <QMutex>
#include <QMutexLocker>
#include <QString>

class UAVObject: public QObject
{
    Q_OBJECT

public:

    /**
     * Object update mode
     */
    typedef enum {
            UPDATEMODE_PERIODIC = 0, /** Automatically update object at periodic intervals */
            UPDATEMODE_ONCHANGE, /** Only update object when its data changes */
            UPDATEMODE_MANUAL,  /** Manually update object, by calling the updated() function */
            UPDATEMODE_NEVER /** Object is never updated */
    } UpdateMode;

    /**
     * Object metadata, each object has a meta object that holds its metadata. The metadata define
     * properties for each object and can be used by multiple modules (e.g. telemetry and logger)
     */
    typedef struct {
            quint8 ackRequired; /** Defines if an ack is required for the transactions of this object (1:acked, 0:not acked) */
            UpdateMode flightTelemetryUpdateMode; /** Update mode used by the autopilot */
            qint32 flightTelemetryUpdatePeriod; /** Update period used by the autopilot (only if telemetry mode is PERIODIC) */
            UpdateMode gcsTelemetryUpdateMode; /** Update mode used by the GCS */
            qint32 gcsTelemetryUpdatePeriod; /** Update period used by the GCS (only if telemetry mode is PERIODIC) */
            UpdateMode loggingUpdateMode; /** Update mode used by the logging module */
            qint32 loggingUpdatePeriod; /** Update period used by the logging module (only if logging mode is PERIODIC) */
    } Metadata;


    UAVObject(quint32 objID, quint32 instID, bool isSingleInst, QString& name, quint32 numBytes);
    quint32 getObjID();
    quint32 getInstID();
    bool isSingleInstance();
    QString getName();
    quint32 getNumBytes(); 
    virtual qint32 pack(quint8* dataOut) = 0;
    virtual qint32 unpack(const quint8* dataIn) = 0;
    virtual void setMetadata(const Metadata& mdata) = 0;
    virtual Metadata getMetadata() = 0;
    void requestUpdate();
    void updated();
    void lock();
    void lock(int timeoutMs);
    void unlock();
    QMutex* getMutex();

signals:
    void objectUpdated(UAVObject* obj, bool unpacked);
    void updateRequested(UAVObject* obj);

protected:
    quint32 objID;
    quint32 instID;
    bool isSingleInst;
    QString name;
    quint32 numBytes;
    QMutex* mutex;

};

#endif // UAVOBJECT_H
