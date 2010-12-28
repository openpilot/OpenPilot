/**
 ******************************************************************************
 *
 * @file       batterysettings.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectsPlugin UAVObjects Plugin
 * @{
 *   
 * @note       Object definition file: batterysettings.xml. 
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
#include "batterysettings.h"
#include "uavobjectfield.h"

const QString BatterySettings::NAME = QString("BatterySettings");
const QString BatterySettings::DESCRIPTION = QString("Battery configuration information.");

/**
 * Constructor
 */
BatterySettings::BatterySettings(): UAVDataObject(OBJID, ISSINGLEINST, ISSETTINGS, NAME)
{
    // Create fields
    QList<UAVObjectField*> fields;
    QStringList BatteryVoltageElemNames;
    BatteryVoltageElemNames.append("0");
    fields.append( new UAVObjectField(QString("BatteryVoltage"), QString("V"), UAVObjectField::FLOAT32, BatteryVoltageElemNames, QStringList()) );
    QStringList BatteryCapacityElemNames;
    BatteryCapacityElemNames.append("0");
    fields.append( new UAVObjectField(QString("BatteryCapacity"), QString("mAh"), UAVObjectField::UINT32, BatteryCapacityElemNames, QStringList()) );
    QStringList BatteryTypeElemNames;
    BatteryTypeElemNames.append("0");
    QStringList BatteryTypeEnumOptions;
    BatteryTypeEnumOptions.append("LiPo");
    BatteryTypeEnumOptions.append("A123");
    BatteryTypeEnumOptions.append("LiCo");
    BatteryTypeEnumOptions.append("LiFeSO4");
    BatteryTypeEnumOptions.append("None");
    fields.append( new UAVObjectField(QString("BatteryType"), QString(""), UAVObjectField::ENUM, BatteryTypeElemNames, BatteryTypeEnumOptions) );
    QStringList CalibrationsElemNames;
    CalibrationsElemNames.append("Voltage");
    CalibrationsElemNames.append("Current");
    fields.append( new UAVObjectField(QString("Calibrations"), QString(""), UAVObjectField::FLOAT32, CalibrationsElemNames, QStringList()) );

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
UAVObject::Metadata BatterySettings::getDefaultMetadata()
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
void BatterySettings::setDefaultFieldValues()
{
    data.BatteryVoltage = 11.1;
    data.BatteryCapacity = 2200;
    data.BatteryType = 0;
    data.Calibrations[0] = 1;
    data.Calibrations[1] = 1;

}

/**
 * Get the object data fields
 */
BatterySettings::DataFields BatterySettings::getData()
{
    QMutexLocker locker(mutex);
    return data;
}

/**
 * Set the object data fields
 */
void BatterySettings::setData(const DataFields& data)
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
UAVDataObject* BatterySettings::clone(quint32 instID)
{
    BatterySettings* obj = new BatterySettings();
    obj->initialize(instID, this->getMetaObject());
    return obj;
}

/**
 * Static function to retrieve an instance of the object.
 */
BatterySettings* BatterySettings::GetInstance(UAVObjectManager* objMngr, quint32 instID)
{
    return dynamic_cast<BatterySettings*>(objMngr->getObject(BatterySettings::OBJID, instID));
}
