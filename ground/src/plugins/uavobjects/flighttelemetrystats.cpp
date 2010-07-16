/**
 ******************************************************************************
 *
 * @file       flighttelemetrystats.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectsPlugin UAVObjects Plugin
 * @{
 *   
 * @note       Object definition file: flighttelemetrystats.xml. 
 *             This is an automatically generated file.
 *             DO NOT modify manually.
 *
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
#include "flighttelemetrystats.h"
#include "uavobjectfield.h"

const QString FlightTelemetryStats::NAME = QString("FlightTelemetryStats");

/**
 * Constructor
 */
FlightTelemetryStats::FlightTelemetryStats(): UAVDataObject(OBJID, ISSINGLEINST, ISSETTINGS, NAME)
{
    // Create fields
    QList<UAVObjectField*> fields;
    QStringList StatusElemNames;
    StatusElemNames.append("0");
    QStringList StatusEnumOptions;
    StatusEnumOptions.append("Disconnected");
    StatusEnumOptions.append("HandshakeReq");
    StatusEnumOptions.append("HandshakeAck");
    StatusEnumOptions.append("Connected");
    fields.append( new UAVObjectField(QString("Status"), QString(""), UAVObjectField::ENUM, StatusElemNames, StatusEnumOptions) );
    QStringList TxDataRateElemNames;
    TxDataRateElemNames.append("0");
    fields.append( new UAVObjectField(QString("TxDataRate"), QString("bytes/sec"), UAVObjectField::FLOAT32, TxDataRateElemNames, QStringList()) );
    QStringList RxDataRateElemNames;
    RxDataRateElemNames.append("0");
    fields.append( new UAVObjectField(QString("RxDataRate"), QString("bytes/sec"), UAVObjectField::FLOAT32, RxDataRateElemNames, QStringList()) );
    QStringList TxFailuresElemNames;
    TxFailuresElemNames.append("0");
    fields.append( new UAVObjectField(QString("TxFailures"), QString("count"), UAVObjectField::UINT32, TxFailuresElemNames, QStringList()) );
    QStringList RxFailuresElemNames;
    RxFailuresElemNames.append("0");
    fields.append( new UAVObjectField(QString("RxFailures"), QString("count"), UAVObjectField::UINT32, RxFailuresElemNames, QStringList()) );
    QStringList TxRetriesElemNames;
    TxRetriesElemNames.append("0");
    fields.append( new UAVObjectField(QString("TxRetries"), QString("count"), UAVObjectField::UINT32, TxRetriesElemNames, QStringList()) );

    // Initialize object
    initializeFields(fields, (quint8*)&data, NUMBYTES);
    // Set the default field values
    setDefaultFieldValues();
}

/**
 * Get the default metadata for this object
 */
UAVObject::Metadata FlightTelemetryStats::getDefaultMetadata()
{
    UAVObject::Metadata metadata;
    metadata.flightAccess = ACCESS_READWRITE;
    metadata.gcsAccess = ACCESS_READWRITE;
    metadata.gcsTelemetryAcked = 1;
    metadata.gcsTelemetryUpdateMode = UAVObject::UPDATEMODE_MANUAL;
    metadata.gcsTelemetryUpdatePeriod = 0;
    metadata.flightTelemetryAcked = 1;
    metadata.flightTelemetryUpdateMode = UAVObject::UPDATEMODE_PERIODIC;
    metadata.flightTelemetryUpdatePeriod = 5000;
    metadata.loggingUpdateMode = UAVObject::UPDATEMODE_PERIODIC;
    metadata.loggingUpdatePeriod = 5000;
    return metadata;
}

/**
 * Initialize object fields with the default values.
 * If a default value is not specified the object fields
 * will be initialized to zero.
 */
void FlightTelemetryStats::setDefaultFieldValues()
{

}

/**
 * Get the object data fields
 */
FlightTelemetryStats::DataFields FlightTelemetryStats::getData()
{
    QMutexLocker locker(mutex);
    return data;
}

/**
 * Set the object data fields
 */
void FlightTelemetryStats::setData(const DataFields& data)
{
    QMutexLocker locker(mutex);
    // Get metadata
    Metadata mdata = getMetadata();
    // Update object if the access mode permits
    if ( mdata.gcsAccess == ACCESS_READWRITE )
    {
        this->data = data;
        emit objectUpdatedAuto(this); // trigger object updated event
        emit objectUpdated(this);
    }
}

/**
 * Create a clone of this object, a new instance ID must be specified.
 * Do not use this function directly to create new instances, the
 * UAVObjectManager should be used instead.
 */
UAVDataObject* FlightTelemetryStats::clone(quint32 instID)
{
    FlightTelemetryStats* obj = new FlightTelemetryStats();
    obj->initialize(instID, this->getMetaObject());
    return obj;
}

/**
 * Static function to retrieve an instance of the object.
 */
FlightTelemetryStats* FlightTelemetryStats::GetInstance(UAVObjectManager* objMngr, quint32 instID)
{
    return dynamic_cast<FlightTelemetryStats*>(objMngr->getObject(FlightTelemetryStats::OBJID, instID));
}
