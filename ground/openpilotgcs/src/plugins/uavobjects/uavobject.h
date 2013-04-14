/**
 ******************************************************************************
 *
 * @file       uavobject.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectsPlugin UAVObjects Plugin
 * @{
 * @brief      The UAVUObjects GCS plugin
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
#ifndef UAVOBJECT_H
#define UAVOBJECT_H

#include "uavobjects_global.h"
#include <QtGlobal>
#include <QObject>
#include <QMutex>
#include <QMutexLocker>
#include <QString>
#include <QList>
#include <QFile>
#include <stdint.h>
#include "uavobjectfield.h"

#define UAVOBJ_ACCESS_SHIFT 0
#define UAVOBJ_GCS_ACCESS_SHIFT 1
#define UAVOBJ_TELEMETRY_ACKED_SHIFT 2
#define UAVOBJ_GCS_TELEMETRY_ACKED_SHIFT 3
#define UAVOBJ_TELEMETRY_UPDATE_MODE_SHIFT 4
#define UAVOBJ_GCS_TELEMETRY_UPDATE_MODE_SHIFT 6
#define UAVOBJ_UPDATE_MODE_MASK 0x3

class UAVObjectField;

class UAVOBJECTS_EXPORT UAVObject: public QObject
{
    Q_OBJECT

public:

    /**
     * Object update mode
     */
    typedef enum {
            UPDATEMODE_MANUAL = 0,  /** Manually update object, by calling the updated() function */
            UPDATEMODE_PERIODIC = 1, /** Automatically update object at periodic intervals */
            UPDATEMODE_ONCHANGE = 2, /** Only update object when its data changes */
            UPDATEMODE_THROTTLED = 3 /** Object is updated on change, but not more often than the interval time */
    } UpdateMode;

    /**
     * Access mode
     */
    typedef enum {
            ACCESS_READWRITE = 0,
            ACCESS_READONLY = 1
    } AccessMode;

    /**
     * Object metadata, each object has a meta object that holds its metadata. The metadata define
     * properties for each object and can be used by multiple modules (e.g. telemetry and logger)
     *
     * The object metadata flags are packed into a single 16 bit integer.
     * The bits in the flag field are defined as:
     *
     *   Bit(s)  Name                       Meaning
     *   ------  ----                       -------
     *      0    access                     Defines the access level for the local transactions (readonly=0 and readwrite=1)
     *      1    gcsAccess                  Defines the access level for the local GCS transactions (readonly=0 and readwrite=1), not used in the flight s/w
     *      2    telemetryAcked             Defines if an ack is required for the transactions of this object (1:acked, 0:not acked)
     *      3    gcsTelemetryAcked          Defines if an ack is required for the transactions of this object (1:acked, 0:not acked)
     *    4-5    telemetryUpdateMode        Update mode used by the telemetry module (UAVObjUpdateMode)
     *    6-7    gcsTelemetryUpdateMode     Update mode used by the GCS (UAVObjUpdateMode)
     */
     typedef struct {
        quint8 flags; /** Defines flags for update and logging modes and whether an update should be ACK'd (bits defined above) */
        quint16 flightTelemetryUpdatePeriod; /** Update period used by the telemetry module (only if telemetry mode is PERIODIC) */
        quint16 gcsTelemetryUpdatePeriod; /** Update period used by the GCS (only if telemetry mode is PERIODIC) */
        quint16 loggingUpdatePeriod; /** Update period used by the logging module (only if logging mode is PERIODIC) */
     } __attribute__((packed)) Metadata;


    UAVObject(quint32 objID, bool isSingleInst, const QString& name);
    void initialize(quint32 instID);
    quint32 getObjID();
    quint32 getInstID();
    bool isSingleInstance();
    QString getName();
    QString getCategory();
    QString getDescription();
    quint32 getNumBytes(); 
    qint32 pack(quint8* dataOut);
    qint32 unpack(const quint8* dataIn);
    bool save();
    bool save(QFile& file);
    bool load();
    bool load(QFile& file);
    virtual void setMetadata(const Metadata& mdata) = 0;
    virtual Metadata getMetadata() = 0;
    virtual Metadata getDefaultMetadata() = 0;
    void lock();
    void lock(int timeoutMs);
    void unlock();
    QMutex* getMutex();
    qint32 getNumFields();
    QList<UAVObjectField*> getFields();
    UAVObjectField* getField(const QString& name);
    QString toString();
    QString toStringBrief();
    QString toStringData();
    void emitTransactionCompleted(bool success);
    void emitNewInstance(UAVObject *);

    // Metadata accessors
    static void MetadataInitialize(Metadata& meta);
    static AccessMode GetFlightAccess(const Metadata& meta);
    static void SetFlightAccess(Metadata& meta, AccessMode mode);
    static AccessMode GetGcsAccess(const Metadata& meta);
    static void SetGcsAccess(Metadata& meta, AccessMode mode);
    static quint8 GetFlightTelemetryAcked(const Metadata& meta);
    static void SetFlightTelemetryAcked(Metadata& meta, quint8 val);
    static quint8 GetGcsTelemetryAcked(const Metadata& meta);
    static void SetGcsTelemetryAcked(Metadata& meta, quint8 val);
    static UpdateMode GetFlightTelemetryUpdateMode(const Metadata& meta);
    static void SetFlightTelemetryUpdateMode(Metadata& meta, UpdateMode val);
    static UpdateMode GetGcsTelemetryUpdateMode(const Metadata& meta);
    static void SetGcsTelemetryUpdateMode(Metadata& meta, UpdateMode val);
		
public slots:
    void requestUpdate();
    void updated();

signals:
    void objectUpdated(UAVObject* obj);
    void objectUpdatedAuto(UAVObject* obj);
    void objectUpdatedManual(UAVObject* obj);
    void objectUpdatedPeriodic(UAVObject* obj);
    void objectUnpacked(UAVObject* obj);
    void updateRequested(UAVObject* obj);
    void transactionCompleted(UAVObject* obj, bool success);
    void newInstance(UAVObject* obj);

private slots:
    void fieldUpdated(UAVObjectField* field);

protected:
    quint32 objID;
    quint32 instID;
    bool isSingleInst;
    QString name;
    QString description;
    QString category;
    quint32 numBytes;
    QMutex* mutex;
    quint8* data;
    QList<UAVObjectField*> fields;

    void initializeFields(QList<UAVObjectField*>& fields, quint8* data, quint32 numBytes);
    void setDescription(const QString& description);
    void setCategory(const QString& category);
};

#endif // UAVOBJECT_H
