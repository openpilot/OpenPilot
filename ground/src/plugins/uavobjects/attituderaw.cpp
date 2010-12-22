/**
 ******************************************************************************
 *
 * @file       attituderaw.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectsPlugin UAVObjects Plugin
 * @{
 *   
 * @note       Object definition file: attituderaw.xml. 
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
#include "attituderaw.h"
#include "uavobjectfield.h"

const QString AttitudeRaw::NAME = QString("AttitudeRaw");
const QString AttitudeRaw::DESCRIPTION = QString("The raw attitude sensor data from @ref AHRSCommsModule.  Not always updated.");

/**
 * Constructor
 */
AttitudeRaw::AttitudeRaw(): UAVDataObject(OBJID, ISSINGLEINST, ISSETTINGS, NAME)
{
    // Create fields
    QList<UAVObjectField*> fields;
    QStringList magnetometersElemNames;
    magnetometersElemNames.append("X");
    magnetometersElemNames.append("Y");
    magnetometersElemNames.append("Z");
    fields.append( new UAVObjectField(QString("magnetometers"), QString("mGa"), UAVObjectField::INT16, magnetometersElemNames, QStringList()) );
    QStringList gyrosElemNames;
    gyrosElemNames.append("X");
    gyrosElemNames.append("Y");
    gyrosElemNames.append("Z");
    fields.append( new UAVObjectField(QString("gyros"), QString("raw"), UAVObjectField::UINT16, gyrosElemNames, QStringList()) );
    QStringList gyros_filteredElemNames;
    gyros_filteredElemNames.append("X");
    gyros_filteredElemNames.append("Y");
    gyros_filteredElemNames.append("Z");
    fields.append( new UAVObjectField(QString("gyros_filtered"), QString("deg/s"), UAVObjectField::FLOAT32, gyros_filteredElemNames, QStringList()) );
    QStringList gyrotempElemNames;
    gyrotempElemNames.append("XY");
    gyrotempElemNames.append("Z");
    fields.append( new UAVObjectField(QString("gyrotemp"), QString("raw"), UAVObjectField::UINT16, gyrotempElemNames, QStringList()) );
    QStringList accelsElemNames;
    accelsElemNames.append("X");
    accelsElemNames.append("Y");
    accelsElemNames.append("Z");
    fields.append( new UAVObjectField(QString("accels"), QString("raw"), UAVObjectField::UINT16, accelsElemNames, QStringList()) );
    QStringList accels_filteredElemNames;
    accels_filteredElemNames.append("X");
    accels_filteredElemNames.append("Y");
    accels_filteredElemNames.append("Z");
    fields.append( new UAVObjectField(QString("accels_filtered"), QString("m/s"), UAVObjectField::FLOAT32, accels_filteredElemNames, QStringList()) );

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
UAVObject::Metadata AttitudeRaw::getDefaultMetadata()
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
void AttitudeRaw::setDefaultFieldValues()
{

}

/**
 * Get the object data fields
 */
AttitudeRaw::DataFields AttitudeRaw::getData()
{
    QMutexLocker locker(mutex);
    return data;
}

/**
 * Set the object data fields
 */
void AttitudeRaw::setData(const DataFields& data)
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
UAVDataObject* AttitudeRaw::clone(quint32 instID)
{
    AttitudeRaw* obj = new AttitudeRaw();
    obj->initialize(instID, this->getMetaObject());
    return obj;
}

/**
 * Static function to retrieve an instance of the object.
 */
AttitudeRaw* AttitudeRaw::GetInstance(UAVObjectManager* objMngr, quint32 instID)
{
    return dynamic_cast<AttitudeRaw*>(objMngr->getObject(AttitudeRaw::OBJID, instID));
}
