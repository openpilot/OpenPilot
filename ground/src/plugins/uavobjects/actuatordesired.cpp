/**
 ******************************************************************************
 *
 * @file       actuatordesired.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectsPlugin UAVObjects Plugin
 * @{
 *   
 * @note       Object definition file: actuatordesired.xml. 
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
#include "actuatordesired.h"
#include "uavobjectfield.h"

const QString ActuatorDesired::NAME = QString("ActuatorDesired");
const QString ActuatorDesired::DESCRIPTION = QString("Desired raw, pitch and yaw actuator settings.  Comes from either @ref StabilizationModule or @ref ManualControlModule depending on FlightMode.");

/**
 * Constructor
 */
ActuatorDesired::ActuatorDesired(): UAVDataObject(OBJID, ISSINGLEINST, ISSETTINGS, NAME)
{
    // Create fields
    QList<UAVObjectField*> fields;
    QStringList RollElemNames;
    RollElemNames.append("0");
    fields.append( new UAVObjectField(QString("Roll"), QString("%"), UAVObjectField::FLOAT32, RollElemNames, QStringList()) );
    QStringList PitchElemNames;
    PitchElemNames.append("0");
    fields.append( new UAVObjectField(QString("Pitch"), QString("%"), UAVObjectField::FLOAT32, PitchElemNames, QStringList()) );
    QStringList YawElemNames;
    YawElemNames.append("0");
    fields.append( new UAVObjectField(QString("Yaw"), QString("%"), UAVObjectField::FLOAT32, YawElemNames, QStringList()) );
    QStringList ThrottleElemNames;
    ThrottleElemNames.append("0");
    fields.append( new UAVObjectField(QString("Throttle"), QString("%"), UAVObjectField::FLOAT32, ThrottleElemNames, QStringList()) );
    QStringList UpdateTimeElemNames;
    UpdateTimeElemNames.append("0");
    fields.append( new UAVObjectField(QString("UpdateTime"), QString("ms"), UAVObjectField::FLOAT32, UpdateTimeElemNames, QStringList()) );
    QStringList NumLongUpdatesElemNames;
    NumLongUpdatesElemNames.append("0");
    fields.append( new UAVObjectField(QString("NumLongUpdates"), QString("ms"), UAVObjectField::FLOAT32, NumLongUpdatesElemNames, QStringList()) );

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
UAVObject::Metadata ActuatorDesired::getDefaultMetadata()
{
    UAVObject::Metadata metadata;
    metadata.flightAccess = ACCESS_READWRITE;
    metadata.gcsAccess = ACCESS_READWRITE;
    metadata.gcsTelemetryAcked = 0;
    metadata.gcsTelemetryUpdateMode = UAVObject::UPDATEMODE_MANUAL;
    metadata.gcsTelemetryUpdatePeriod = 0;
    metadata.flightTelemetryAcked = 0;
    metadata.flightTelemetryUpdateMode = UAVObject::UPDATEMODE_PERIODIC;
    metadata.flightTelemetryUpdatePeriod = 1000;
    metadata.loggingUpdateMode = UAVObject::UPDATEMODE_NEVER;
    metadata.loggingUpdatePeriod = 0;
    return metadata;
}

/**
 * Initialize object fields with the default values.
 * If a default value is not specified the object fields
 * will be initialized to zero.
 */
void ActuatorDesired::setDefaultFieldValues()
{

}

/**
 * Get the object data fields
 */
ActuatorDesired::DataFields ActuatorDesired::getData()
{
    QMutexLocker locker(mutex);
    return data;
}

/**
 * Set the object data fields
 */
void ActuatorDesired::setData(const DataFields& data)
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
UAVDataObject* ActuatorDesired::clone(quint32 instID)
{
    ActuatorDesired* obj = new ActuatorDesired();
    obj->initialize(instID, this->getMetaObject());
    return obj;
}

/**
 * Static function to retrieve an instance of the object.
 */
ActuatorDesired* ActuatorDesired::GetInstance(UAVObjectManager* objMngr, quint32 instID)
{
    return dynamic_cast<ActuatorDesired*>(objMngr->getObject(ActuatorDesired::OBJID, instID));
}
