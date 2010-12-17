/**
 ******************************************************************************
 *
 * @file       i2cstats.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectsPlugin UAVObjects Plugin
 * @{
 *   
 * @note       Object definition file: i2cstats.xml. 
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
#include "i2cstats.h"
#include "uavobjectfield.h"

const QString I2CStats::NAME = QString("I2CStats");

/**
 * Constructor
 */
I2CStats::I2CStats(): UAVDataObject(OBJID, ISSINGLEINST, ISSETTINGS, NAME)
{
    // Create fields
    QList<UAVObjectField*> fields;
    QStringList event_errorsElemNames;
    event_errorsElemNames.append("0");
    fields.append( new UAVObjectField(QString("event_errors"), QString(""), UAVObjectField::UINT16, event_errorsElemNames, QStringList()) );
    QStringList fsm_errorsElemNames;
    fsm_errorsElemNames.append("0");
    fields.append( new UAVObjectField(QString("fsm_errors"), QString(""), UAVObjectField::UINT16, fsm_errorsElemNames, QStringList()) );
    QStringList irq_errorsElemNames;
    irq_errorsElemNames.append("0");
    fields.append( new UAVObjectField(QString("irq_errors"), QString(""), UAVObjectField::UINT16, irq_errorsElemNames, QStringList()) );
    QStringList last_error_typeElemNames;
    last_error_typeElemNames.append("0");
    QStringList last_error_typeEnumOptions;
    last_error_typeEnumOptions.append("EVENT");
    last_error_typeEnumOptions.append("FSM");
    last_error_typeEnumOptions.append("INTERRUPT");
    fields.append( new UAVObjectField(QString("last_error_type"), QString(""), UAVObjectField::ENUM, last_error_typeElemNames, last_error_typeEnumOptions) );
    QStringList event_logElemNames;
    event_logElemNames.append("0");
    event_logElemNames.append("1");
    event_logElemNames.append("2");
    event_logElemNames.append("3");
    event_logElemNames.append("4");
    fields.append( new UAVObjectField(QString("event_log"), QString(""), UAVObjectField::UINT32, event_logElemNames, QStringList()) );
    QStringList state_logElemNames;
    state_logElemNames.append("0");
    state_logElemNames.append("1");
    state_logElemNames.append("2");
    state_logElemNames.append("3");
    state_logElemNames.append("4");
    fields.append( new UAVObjectField(QString("state_log"), QString(""), UAVObjectField::UINT32, state_logElemNames, QStringList()) );

    // Initialize object
    initializeFields(fields, (quint8*)&data, NUMBYTES);
    // Set the default field values
    setDefaultFieldValues();
}

/**
 * Get the default metadata for this object
 */
UAVObject::Metadata I2CStats::getDefaultMetadata()
{
    UAVObject::Metadata metadata;
    metadata.flightAccess = ACCESS_READWRITE;
    metadata.gcsAccess = ACCESS_READWRITE;
    metadata.gcsTelemetryAcked = 0;
    metadata.gcsTelemetryUpdateMode = UAVObject::UPDATEMODE_MANUAL;
    metadata.gcsTelemetryUpdatePeriod = 0;
    metadata.flightTelemetryAcked = 0;
    metadata.flightTelemetryUpdateMode = UAVObject::UPDATEMODE_PERIODIC;
    metadata.flightTelemetryUpdatePeriod = 10000;
    metadata.loggingUpdateMode = UAVObject::UPDATEMODE_PERIODIC;
    metadata.loggingUpdatePeriod = 30000;
    return metadata;
}

/**
 * Initialize object fields with the default values.
 * If a default value is not specified the object fields
 * will be initialized to zero.
 */
void I2CStats::setDefaultFieldValues()
{

}

/**
 * Get the object data fields
 */
I2CStats::DataFields I2CStats::getData()
{
    QMutexLocker locker(mutex);
    return data;
}

/**
 * Set the object data fields
 */
void I2CStats::setData(const DataFields& data)
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
UAVDataObject* I2CStats::clone(quint32 instID)
{
    I2CStats* obj = new I2CStats();
    obj->initialize(instID, this->getMetaObject());
    return obj;
}

/**
 * Static function to retrieve an instance of the object.
 */
I2CStats* I2CStats::GetInstance(UAVObjectManager* objMngr, quint32 instID)
{
    return dynamic_cast<I2CStats*>(objMngr->getObject(I2CStats::OBJID, instID));
}
