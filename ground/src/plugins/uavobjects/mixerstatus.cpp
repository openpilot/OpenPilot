/**
 ******************************************************************************
 *
 * @file       mixerstatus.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectsPlugin UAVObjects Plugin
 * @{
 *   
 * @note       Object definition file: mixerstatus.xml. 
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
#include "mixerstatus.h"
#include "uavobjectfield.h"

const QString MixerStatus::NAME = QString("MixerStatus");
const QString MixerStatus::DESCRIPTION = QString("Status for the matrix mixer showing the output of each mixer after all scaling");

/**
 * Constructor
 */
MixerStatus::MixerStatus(): UAVDataObject(OBJID, ISSINGLEINST, ISSETTINGS, NAME)
{
    // Create fields
    QList<UAVObjectField*> fields;
    QStringList Mixer1ElemNames;
    Mixer1ElemNames.append("0");
    fields.append( new UAVObjectField(QString("Mixer1"), QString(""), UAVObjectField::FLOAT32, Mixer1ElemNames, QStringList()) );
    QStringList Mixer2ElemNames;
    Mixer2ElemNames.append("0");
    fields.append( new UAVObjectField(QString("Mixer2"), QString(""), UAVObjectField::FLOAT32, Mixer2ElemNames, QStringList()) );
    QStringList Mixer3ElemNames;
    Mixer3ElemNames.append("0");
    fields.append( new UAVObjectField(QString("Mixer3"), QString(""), UAVObjectField::FLOAT32, Mixer3ElemNames, QStringList()) );
    QStringList Mixer4ElemNames;
    Mixer4ElemNames.append("0");
    fields.append( new UAVObjectField(QString("Mixer4"), QString(""), UAVObjectField::FLOAT32, Mixer4ElemNames, QStringList()) );
    QStringList Mixer5ElemNames;
    Mixer5ElemNames.append("0");
    fields.append( new UAVObjectField(QString("Mixer5"), QString(""), UAVObjectField::FLOAT32, Mixer5ElemNames, QStringList()) );
    QStringList Mixer6ElemNames;
    Mixer6ElemNames.append("0");
    fields.append( new UAVObjectField(QString("Mixer6"), QString(""), UAVObjectField::FLOAT32, Mixer6ElemNames, QStringList()) );
    QStringList Mixer7ElemNames;
    Mixer7ElemNames.append("0");
    fields.append( new UAVObjectField(QString("Mixer7"), QString(""), UAVObjectField::FLOAT32, Mixer7ElemNames, QStringList()) );
    QStringList Mixer8ElemNames;
    Mixer8ElemNames.append("0");
    fields.append( new UAVObjectField(QString("Mixer8"), QString(""), UAVObjectField::FLOAT32, Mixer8ElemNames, QStringList()) );

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
UAVObject::Metadata MixerStatus::getDefaultMetadata()
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
    metadata.loggingUpdateMode = UAVObject::UPDATEMODE_PERIODIC;
    metadata.loggingUpdatePeriod = 1000;
    return metadata;
}

/**
 * Initialize object fields with the default values.
 * If a default value is not specified the object fields
 * will be initialized to zero.
 */
void MixerStatus::setDefaultFieldValues()
{

}

/**
 * Get the object data fields
 */
MixerStatus::DataFields MixerStatus::getData()
{
    QMutexLocker locker(mutex);
    return data;
}

/**
 * Set the object data fields
 */
void MixerStatus::setData(const DataFields& data)
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
UAVDataObject* MixerStatus::clone(quint32 instID)
{
    MixerStatus* obj = new MixerStatus();
    obj->initialize(instID, this->getMetaObject());
    return obj;
}

/**
 * Static function to retrieve an instance of the object.
 */
MixerStatus* MixerStatus::GetInstance(UAVObjectManager* objMngr, quint32 instID)
{
    return dynamic_cast<MixerStatus*>(objMngr->getObject(MixerStatus::OBJID, instID));
}
