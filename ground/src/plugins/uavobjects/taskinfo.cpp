/**
 ******************************************************************************
 *
 * @file       taskinfo.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectsPlugin UAVObjects Plugin
 * @{
 *   
 * @note       Object definition file: taskinfo.xml. 
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
#include "taskinfo.h"
#include "uavobjectfield.h"

const QString TaskInfo::NAME = QString("TaskInfo");
const QString TaskInfo::DESCRIPTION = QString("Task information");

/**
 * Constructor
 */
TaskInfo::TaskInfo(): UAVDataObject(OBJID, ISSINGLEINST, ISSETTINGS, NAME)
{
    // Create fields
    QList<UAVObjectField*> fields;
    QStringList StackRemainingElemNames;
    StackRemainingElemNames.append("System");
    StackRemainingElemNames.append("Actuator");
    StackRemainingElemNames.append("TelemetryTx");
    StackRemainingElemNames.append("TelemetryTxPri");
    StackRemainingElemNames.append("TelemetryRx");
    StackRemainingElemNames.append("GPS");
    StackRemainingElemNames.append("ManualControl");
    StackRemainingElemNames.append("Altitude");
    StackRemainingElemNames.append("AHRSComms");
    StackRemainingElemNames.append("Stabilization");
    StackRemainingElemNames.append("Guidance");
    StackRemainingElemNames.append("Watchdog");
    fields.append( new UAVObjectField(QString("StackRemaining"), QString("bytes"), UAVObjectField::UINT16, StackRemainingElemNames, QStringList()) );
    QStringList RunningElemNames;
    RunningElemNames.append("System");
    RunningElemNames.append("Actuator");
    RunningElemNames.append("TelemetryTx");
    RunningElemNames.append("TelemetryTxPri");
    RunningElemNames.append("TelemetryRx");
    RunningElemNames.append("GPS");
    RunningElemNames.append("ManualControl");
    RunningElemNames.append("Altitude");
    RunningElemNames.append("AHRSComms");
    RunningElemNames.append("Stabilization");
    RunningElemNames.append("Guidance");
    RunningElemNames.append("Watchdog");
    QStringList RunningEnumOptions;
    RunningEnumOptions.append("False");
    RunningEnumOptions.append("True");
    fields.append( new UAVObjectField(QString("Running"), QString("bool"), UAVObjectField::ENUM, RunningElemNames, RunningEnumOptions) );

    // Initialize object
    initializeFields(fields, (quint8*)&data, NUMBYTES);
    // Set the default field values
    setDefaultFieldValues();
    // Set the object description
    setDescription(DESCRIPTION);
}

/**
 * Get the default metadata for this object
 */
UAVObject::Metadata TaskInfo::getDefaultMetadata()
{
    UAVObject::Metadata metadata;
    metadata.flightAccess = ACCESS_READWRITE;
    metadata.gcsAccess = ACCESS_READWRITE;
    metadata.gcsTelemetryAcked = 1;
    metadata.gcsTelemetryUpdateMode = UAVObject::UPDATEMODE_ONCHANGE;
    metadata.gcsTelemetryUpdatePeriod = 0;
    metadata.flightTelemetryAcked = 1;
    metadata.flightTelemetryUpdateMode = UAVObject::UPDATEMODE_PERIODIC;
    metadata.flightTelemetryUpdatePeriod = 10000;
    metadata.loggingUpdateMode = UAVObject::UPDATEMODE_PERIODIC;
    metadata.loggingUpdatePeriod = 1000;
    return metadata;
}

/**
 * Initialize object fields with the default values.
 * If a default value is not specified the object fields
 * will be initialized to zero.
 */
void TaskInfo::setDefaultFieldValues()
{

}

/**
 * Get the object data fields
 */
TaskInfo::DataFields TaskInfo::getData()
{
    QMutexLocker locker(mutex);
    return data;
}

/**
 * Set the object data fields
 */
void TaskInfo::setData(const DataFields& data)
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
UAVDataObject* TaskInfo::clone(quint32 instID)
{
    TaskInfo* obj = new TaskInfo();
    obj->initialize(instID, this->getMetaObject());
    return obj;
}

/**
 * Static function to retrieve an instance of the object.
 */
TaskInfo* TaskInfo::GetInstance(UAVObjectManager* objMngr, quint32 instID)
{
    return dynamic_cast<TaskInfo*>(objMngr->getObject(TaskInfo::OBJID, instID));
}
