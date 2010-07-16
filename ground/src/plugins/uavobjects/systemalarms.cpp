/**
 ******************************************************************************
 *
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectsPlugin UAVObjects Plugin
 * @{
 * @brief      The UAVUObjects GCS plugin 
 *   
 * @note       Object definition file: systemalarms.xml. 
 *             This is an automatically generated file.
 *             DO NOT modify manually.
 *
 * @file       systemalarms.cpp
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
#include "systemalarms.h"
#include "uavobjectfield.h"

const QString SystemAlarms::NAME = QString("SystemAlarms");

/**
 * Constructor
 */
SystemAlarms::SystemAlarms(): UAVDataObject(OBJID, ISSINGLEINST, ISSETTINGS, NAME)
{
    // Create fields
    QList<UAVObjectField*> fields;
    QStringList AlarmElemNames;
    AlarmElemNames.append("OutOfMemory");
    AlarmElemNames.append("StackOverflow");
    AlarmElemNames.append("CPUOverload");
    AlarmElemNames.append("EventSystem");
    AlarmElemNames.append("SDCard");
    AlarmElemNames.append("Telemetry");
    AlarmElemNames.append("ManualControl");
    AlarmElemNames.append("Actuator");
    AlarmElemNames.append("Stabilization");
    AlarmElemNames.append("AHRSComms");
    QStringList AlarmEnumOptions;
    AlarmEnumOptions.append("OK");
    AlarmEnumOptions.append("Warning");
    AlarmEnumOptions.append("Error");
    AlarmEnumOptions.append("Critical");
    fields.append( new UAVObjectField(QString("Alarm"), QString(""), UAVObjectField::ENUM, AlarmElemNames, AlarmEnumOptions) );

    // Initialize object
    initializeFields(fields, (quint8*)&data, NUMBYTES);
    // Set the default field values
    setDefaultFieldValues();
}

/**
 * Get the default metadata for this object
 */
UAVObject::Metadata SystemAlarms::getDefaultMetadata()
{
    UAVObject::Metadata metadata;
    metadata.flightAccess = ACCESS_READWRITE;
    metadata.gcsAccess = ACCESS_READWRITE;
    metadata.gcsTelemetryAcked = 1;
    metadata.gcsTelemetryUpdateMode = UAVObject::UPDATEMODE_ONCHANGE;
    metadata.gcsTelemetryUpdatePeriod = 0;
    metadata.flightTelemetryAcked = 1;
    metadata.flightTelemetryUpdateMode = UAVObject::UPDATEMODE_PERIODIC;
    metadata.flightTelemetryUpdatePeriod = 4000;
    metadata.loggingUpdateMode = UAVObject::UPDATEMODE_PERIODIC;
    metadata.loggingUpdatePeriod = 1000;
    return metadata;
}

/**
 * Initialize object fields with the default values.
 * If a default value is not specified the object fields
 * will be initialized to zero.
 */
void SystemAlarms::setDefaultFieldValues()
{

}

/**
 * Get the object data fields
 */
SystemAlarms::DataFields SystemAlarms::getData()
{
    QMutexLocker locker(mutex);
    return data;
}

/**
 * Set the object data fields
 */
void SystemAlarms::setData(const DataFields& data)
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
UAVDataObject* SystemAlarms::clone(quint32 instID)
{
    SystemAlarms* obj = new SystemAlarms();
    obj->initialize(instID, this->getMetaObject());
    return obj;
}

/**
 * Static function to retrieve an instance of the object.
 */
SystemAlarms* SystemAlarms::GetInstance(UAVObjectManager* objMngr, quint32 instID)
{
    return dynamic_cast<SystemAlarms*>(objMngr->getObject(SystemAlarms::OBJID, instID));
}
