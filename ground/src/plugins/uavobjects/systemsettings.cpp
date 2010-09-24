/**
 ******************************************************************************
 *
 * @file       systemsettings.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectsPlugin UAVObjects Plugin
 * @{
 *   
 * @note       Object definition file: systemsettings.xml. 
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
#include "systemsettings.h"
#include "uavobjectfield.h"

const QString SystemSettings::NAME = QString("SystemSettings");

/**
 * Constructor
 */
SystemSettings::SystemSettings(): UAVDataObject(OBJID, ISSINGLEINST, ISSETTINGS, NAME)
{
    // Create fields
    QList<UAVObjectField*> fields;
    QStringList AirframeTypeElemNames;
    AirframeTypeElemNames.append("0");
    QStringList AirframeTypeEnumOptions;
    AirframeTypeEnumOptions.append("FixedWing");
    AirframeTypeEnumOptions.append("FixedWingElevon");
    AirframeTypeEnumOptions.append("FixedWingVtail");
    AirframeTypeEnumOptions.append("VTOL");
    AirframeTypeEnumOptions.append("HeliCP");
    AirframeTypeEnumOptions.append("QuadX");
    AirframeTypeEnumOptions.append("QuadP");
    AirframeTypeEnumOptions.append("Hexa");
    AirframeTypeEnumOptions.append("Octo");
    AirframeTypeEnumOptions.append("Custom");
    fields.append( new UAVObjectField(QString("AirframeType"), QString(""), UAVObjectField::ENUM, AirframeTypeElemNames, AirframeTypeEnumOptions) );

    // Initialize object
    initializeFields(fields, (quint8*)&data, NUMBYTES);
    // Set the default field values
    setDefaultFieldValues();
}

/**
 * Get the default metadata for this object
 */
UAVObject::Metadata SystemSettings::getDefaultMetadata()
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
void SystemSettings::setDefaultFieldValues()
{
    data.AirframeType = 0;

}

/**
 * Get the object data fields
 */
SystemSettings::DataFields SystemSettings::getData()
{
    QMutexLocker locker(mutex);
    return data;
}

/**
 * Set the object data fields
 */
void SystemSettings::setData(const DataFields& data)
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
UAVDataObject* SystemSettings::clone(quint32 instID)
{
    SystemSettings* obj = new SystemSettings();
    obj->initialize(instID, this->getMetaObject());
    return obj;
}

/**
 * Static function to retrieve an instance of the object.
 */
SystemSettings* SystemSettings::GetInstance(UAVObjectManager* objMngr, quint32 instID)
{
    return dynamic_cast<SystemSettings*>(objMngr->getObject(SystemSettings::OBJID, instID));
}
