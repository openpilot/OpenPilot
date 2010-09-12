/**
 ******************************************************************************
 *
 * @file       vtolsettings.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectsPlugin UAVObjects Plugin
 * @{
 *   
 * @note       Object definition file: vtolsettings.xml. 
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
#include "vtolsettings.h"
#include "uavobjectfield.h"

const QString VTOLSettings::NAME = QString("VTOLSettings");

/**
 * Constructor
 */
VTOLSettings::VTOLSettings(): UAVDataObject(OBJID, ISSINGLEINST, ISSETTINGS, NAME)
{
    // Create fields
    QList<UAVObjectField*> fields;
    QStringList MotorNElemNames;
    MotorNElemNames.append("Throttle");
    MotorNElemNames.append("Roll");
    MotorNElemNames.append("Pitch");
    MotorNElemNames.append("Yaw");
    fields.append( new UAVObjectField(QString("MotorN"), QString(""), UAVObjectField::FLOAT32, MotorNElemNames, QStringList()) );
    QStringList MotorNEElemNames;
    MotorNEElemNames.append("Throttle");
    MotorNEElemNames.append("Roll");
    MotorNEElemNames.append("Pitch");
    MotorNEElemNames.append("Yaw");
    fields.append( new UAVObjectField(QString("MotorNE"), QString(""), UAVObjectField::FLOAT32, MotorNEElemNames, QStringList()) );
    QStringList MotorEElemNames;
    MotorEElemNames.append("Throttle");
    MotorEElemNames.append("Roll");
    MotorEElemNames.append("Pitch");
    MotorEElemNames.append("Yaw");
    fields.append( new UAVObjectField(QString("MotorE"), QString(""), UAVObjectField::FLOAT32, MotorEElemNames, QStringList()) );
    QStringList MotorSEElemNames;
    MotorSEElemNames.append("Throttle");
    MotorSEElemNames.append("Roll");
    MotorSEElemNames.append("Pitch");
    MotorSEElemNames.append("Yaw");
    fields.append( new UAVObjectField(QString("MotorSE"), QString(""), UAVObjectField::FLOAT32, MotorSEElemNames, QStringList()) );
    QStringList MotorSElemNames;
    MotorSElemNames.append("Throttle");
    MotorSElemNames.append("Roll");
    MotorSElemNames.append("Pitch");
    MotorSElemNames.append("Yaw");
    fields.append( new UAVObjectField(QString("MotorS"), QString(""), UAVObjectField::FLOAT32, MotorSElemNames, QStringList()) );
    QStringList MotorSWElemNames;
    MotorSWElemNames.append("Throttle");
    MotorSWElemNames.append("Roll");
    MotorSWElemNames.append("Pitch");
    MotorSWElemNames.append("Yaw");
    fields.append( new UAVObjectField(QString("MotorSW"), QString(""), UAVObjectField::FLOAT32, MotorSWElemNames, QStringList()) );
    QStringList MotorWElemNames;
    MotorWElemNames.append("Throttle");
    MotorWElemNames.append("Roll");
    MotorWElemNames.append("Pitch");
    MotorWElemNames.append("Yaw");
    fields.append( new UAVObjectField(QString("MotorW"), QString(""), UAVObjectField::FLOAT32, MotorWElemNames, QStringList()) );
    QStringList MotorNWElemNames;
    MotorNWElemNames.append("Throttle");
    MotorNWElemNames.append("Roll");
    MotorNWElemNames.append("Pitch");
    MotorNWElemNames.append("Yaw");
    fields.append( new UAVObjectField(QString("MotorNW"), QString(""), UAVObjectField::FLOAT32, MotorNWElemNames, QStringList()) );

    // Initialize object
    initializeFields(fields, (quint8*)&data, NUMBYTES);
    // Set the default field values
    setDefaultFieldValues();
}

/**
 * Get the default metadata for this object
 */
UAVObject::Metadata VTOLSettings::getDefaultMetadata()
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
void VTOLSettings::setDefaultFieldValues()
{
    data.MotorN[0] = 0;
    data.MotorN[1] = 0;
    data.MotorN[2] = 0;
    data.MotorN[3] = 0;
    data.MotorNE[0] = 0;
    data.MotorNE[1] = 0;
    data.MotorNE[2] = 0;
    data.MotorNE[3] = 0;
    data.MotorE[0] = 0;
    data.MotorE[1] = 0;
    data.MotorE[2] = 0;
    data.MotorE[3] = 0;
    data.MotorSE[0] = 0;
    data.MotorSE[1] = 0;
    data.MotorSE[2] = 0;
    data.MotorSE[3] = 0;
    data.MotorS[0] = 0;
    data.MotorS[1] = 0;
    data.MotorS[2] = 0;
    data.MotorS[3] = 0;
    data.MotorSW[0] = 0;
    data.MotorSW[1] = 0;
    data.MotorSW[2] = 0;
    data.MotorSW[3] = 0;
    data.MotorW[0] = 0;
    data.MotorW[1] = 0;
    data.MotorW[2] = 0;
    data.MotorW[3] = 0;
    data.MotorNW[0] = 0;
    data.MotorNW[1] = 0;
    data.MotorNW[2] = 0;
    data.MotorNW[3] = 0;

}

/**
 * Get the object data fields
 */
VTOLSettings::DataFields VTOLSettings::getData()
{
    QMutexLocker locker(mutex);
    return data;
}

/**
 * Set the object data fields
 */
void VTOLSettings::setData(const DataFields& data)
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
UAVDataObject* VTOLSettings::clone(quint32 instID)
{
    VTOLSettings* obj = new VTOLSettings();
    obj->initialize(instID, this->getMetaObject());
    return obj;
}

/**
 * Static function to retrieve an instance of the object.
 */
VTOLSettings* VTOLSettings::GetInstance(UAVObjectManager* objMngr, quint32 instID)
{
    return dynamic_cast<VTOLSettings*>(objMngr->getObject(VTOLSettings::OBJID, instID));
}
