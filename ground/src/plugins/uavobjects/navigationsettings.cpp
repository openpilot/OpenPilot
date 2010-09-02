/**
 ******************************************************************************
 *
 * @file       navigationsettings.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectsPlugin UAVObjects Plugin
 * @{
 *   
 * @note       Object definition file: navigationsettings.xml. 
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
#include "navigationsettings.h"
#include "uavobjectfield.h"

const QString NavigationSettings::NAME = QString("NavigationSettings");

/**
 * Constructor
 */
NavigationSettings::NavigationSettings(): UAVDataObject(OBJID, ISSINGLEINST, ISSETTINGS, NAME)
{
    // Create fields
    QList<UAVObjectField*> fields;
    QStringList UpdatePeriodElemNames;
    UpdatePeriodElemNames.append("0");
    fields.append( new UAVObjectField(QString("UpdatePeriod"), QString("ms"), UAVObjectField::UINT16, UpdatePeriodElemNames, QStringList()) );
    QStringList AccelerationMaxElemNames;
    AccelerationMaxElemNames.append("0");
    fields.append( new UAVObjectField(QString("AccelerationMax"), QString("m/s²"), UAVObjectField::FLOAT32, AccelerationMaxElemNames, QStringList()) );
    QStringList SpeedMaxElemNames;
    SpeedMaxElemNames.append("0");
    fields.append( new UAVObjectField(QString("SpeedMax"), QString("m/s"), UAVObjectField::FLOAT32, SpeedMaxElemNames, QStringList()) );
    QStringList SpeedSafeElemNames;
    SpeedSafeElemNames.append("0");
    fields.append( new UAVObjectField(QString("SpeedSafe"), QString("m/s"), UAVObjectField::FLOAT32, SpeedSafeElemNames, QStringList()) );
    QStringList SpeedMinElemNames;
    SpeedMinElemNames.append("0");
    fields.append( new UAVObjectField(QString("SpeedMin"), QString("m/s"), UAVObjectField::FLOAT32, SpeedMinElemNames, QStringList()) );
    QStringList SettleTimeElemNames;
    SettleTimeElemNames.append("0");
    fields.append( new UAVObjectField(QString("SettleTime"), QString("seconds"), UAVObjectField::FLOAT32, SettleTimeElemNames, QStringList()) );
    QStringList TurnSpeedFactorElemNames;
    TurnSpeedFactorElemNames.append("0");
    fields.append( new UAVObjectField(QString("TurnSpeedFactor"), QString(""), UAVObjectField::FLOAT32, TurnSpeedFactorElemNames, QStringList()) );
    QStringList StabilizationForceFactorElemNames;
    StabilizationForceFactorElemNames.append("0");
    fields.append( new UAVObjectField(QString("StabilizationForceFactor"), QString(""), UAVObjectField::FLOAT32, StabilizationForceFactorElemNames, QStringList()) );

    // Initialize object
    initializeFields(fields, (quint8*)&data, NUMBYTES);
    // Set the default field values
    setDefaultFieldValues();
}

/**
 * Get the default metadata for this object
 */
UAVObject::Metadata NavigationSettings::getDefaultMetadata()
{
    UAVObject::Metadata metadata;
    metadata.flightAccess = ACCESS_READWRITE;
    metadata.gcsAccess = ACCESS_READWRITE;
    metadata.gcsTelemetryAcked = 1;
    metadata.gcsTelemetryUpdateMode = UAVObject::UPDATEMODE_ONCHANGE;
    metadata.gcsTelemetryUpdatePeriod = 0;
    metadata.flightTelemetryAcked = 1;
    metadata.flightTelemetryUpdateMode = UAVObject::UPDATEMODE_ONCHANGE;
    metadata.flightTelemetryUpdatePeriod = 0;
    metadata.loggingUpdateMode = UAVObject::UPDATEMODE_NEVER;
    metadata.loggingUpdatePeriod = 0;
    return metadata;
}

/**
 * Initialize object fields with the default values.
 * If a default value is not specified the object fields
 * will be initialized to zero.
 */
void NavigationSettings::setDefaultFieldValues()
{
    data.UpdatePeriod = 100;
    data.AccelerationMax = 35;
    data.SpeedMax = 300;
    data.SpeedSafe = 100;
    data.SpeedMin = 70;
    data.SettleTime = 12;
    data.TurnSpeedFactor = 0.1;
    data.StabilizationForceFactor = 4;

}

/**
 * Get the object data fields
 */
NavigationSettings::DataFields NavigationSettings::getData()
{
    QMutexLocker locker(mutex);
    return data;
}

/**
 * Set the object data fields
 */
void NavigationSettings::setData(const DataFields& data)
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
UAVDataObject* NavigationSettings::clone(quint32 instID)
{
    NavigationSettings* obj = new NavigationSettings();
    obj->initialize(instID, this->getMetaObject());
    return obj;
}

/**
 * Static function to retrieve an instance of the object.
 */
NavigationSettings* NavigationSettings::GetInstance(UAVObjectManager* objMngr, quint32 instID)
{
    return dynamic_cast<NavigationSettings*>(objMngr->getObject(NavigationSettings::OBJID, instID));
}
