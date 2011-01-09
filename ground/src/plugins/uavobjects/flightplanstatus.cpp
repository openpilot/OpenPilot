/**
 ******************************************************************************
 *
 * @file       flightplanstatus.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectsPlugin UAVObjects Plugin
 * @{
 *   
 * @note       Object definition file: flightplanstatus.xml. 
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
#include "flightplanstatus.h"
#include "uavobjectfield.h"

const QString FlightPlanStatus::NAME = QString("FlightPlanStatus");
const QString FlightPlanStatus::DESCRIPTION = QString("Status of the flight plan script");

/**
 * Constructor
 */
FlightPlanStatus::FlightPlanStatus(): UAVDataObject(OBJID, ISSINGLEINST, ISSETTINGS, NAME)
{
    // Create fields
    QList<UAVObjectField*> fields;
    QStringList StatusElemNames;
    StatusElemNames.append("0");
    QStringList StatusEnumOptions;
    StatusEnumOptions.append("None");
    StatusEnumOptions.append("Running");
    StatusEnumOptions.append("Idle");
    StatusEnumOptions.append("VMInitError");
    StatusEnumOptions.append("ScriptStartError");
    StatusEnumOptions.append("RunTimeError");
    fields.append( new UAVObjectField(QString("Status"), QString(""), UAVObjectField::ENUM, StatusElemNames, StatusEnumOptions) );
    QStringList ErrorTypeElemNames;
    ErrorTypeElemNames.append("0");
    QStringList ErrorTypeEnumOptions;
    ErrorTypeEnumOptions.append("None");
    fields.append( new UAVObjectField(QString("ErrorType"), QString(""), UAVObjectField::ENUM, ErrorTypeElemNames, ErrorTypeEnumOptions) );
    QStringList ErrorFileIDElemNames;
    ErrorFileIDElemNames.append("0");
    fields.append( new UAVObjectField(QString("ErrorFileID"), QString(""), UAVObjectField::UINT32, ErrorFileIDElemNames, QStringList()) );
    QStringList ErrorLineNumElemNames;
    ErrorLineNumElemNames.append("0");
    fields.append( new UAVObjectField(QString("ErrorLineNum"), QString(""), UAVObjectField::UINT32, ErrorLineNumElemNames, QStringList()) );
    QStringList DebugElemNames;
    DebugElemNames.append("0");
    fields.append( new UAVObjectField(QString("Debug"), QString(""), UAVObjectField::FLOAT32, DebugElemNames, QStringList()) );

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
UAVObject::Metadata FlightPlanStatus::getDefaultMetadata()
{
    UAVObject::Metadata metadata;
    metadata.flightAccess = ACCESS_READWRITE;
    metadata.gcsAccess = ACCESS_READWRITE;
    metadata.gcsTelemetryAcked = 0;
    metadata.gcsTelemetryUpdateMode = UAVObject::UPDATEMODE_MANUAL;
    metadata.gcsTelemetryUpdatePeriod = 0;
    metadata.flightTelemetryAcked = 0;
    metadata.flightTelemetryUpdateMode = UAVObject::UPDATEMODE_PERIODIC;
    metadata.flightTelemetryUpdatePeriod = 2000;
    metadata.loggingUpdateMode = UAVObject::UPDATEMODE_NEVER;
    metadata.loggingUpdatePeriod = 0;
    return metadata;
}

/**
 * Initialize object fields with the default values.
 * If a default value is not specified the object fields
 * will be initialized to zero.
 */
void FlightPlanStatus::setDefaultFieldValues()
{
    data.Status = 0;
    data.ErrorType = 0;

}

/**
 * Get the object data fields
 */
FlightPlanStatus::DataFields FlightPlanStatus::getData()
{
    QMutexLocker locker(mutex);
    return data;
}

/**
 * Set the object data fields
 */
void FlightPlanStatus::setData(const DataFields& data)
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
UAVDataObject* FlightPlanStatus::clone(quint32 instID)
{
    FlightPlanStatus* obj = new FlightPlanStatus();
    obj->initialize(instID, this->getMetaObject());
    return obj;
}

/**
 * Static function to retrieve an instance of the object.
 */
FlightPlanStatus* FlightPlanStatus::GetInstance(UAVObjectManager* objMngr, quint32 instID)
{
    return dynamic_cast<FlightPlanStatus*>(objMngr->getObject(FlightPlanStatus::OBJID, instID));
}
