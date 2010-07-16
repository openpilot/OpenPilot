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
 * @note       Object definition file: altitudeactual.xml. 
 *             This is an automatically generated file.
 *             DO NOT modify manually.
 *
 * @file       altitudeactual.cpp
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
#include "altitudeactual.h"
#include "uavobjectfield.h"

const QString AltitudeActual::NAME = QString("AltitudeActual");

/**
 * Constructor
 */
AltitudeActual::AltitudeActual(): UAVDataObject(OBJID, ISSINGLEINST, ISSETTINGS, NAME)
{
    // Create fields
    QList<UAVObjectField*> fields;
    QStringList AltitudeElemNames;
    AltitudeElemNames.append("0");
    fields.append( new UAVObjectField(QString("Altitude"), QString("m"), UAVObjectField::FLOAT32, AltitudeElemNames, QStringList()) );
    QStringList TemperatureElemNames;
    TemperatureElemNames.append("0");
    fields.append( new UAVObjectField(QString("Temperature"), QString("C"), UAVObjectField::FLOAT32, TemperatureElemNames, QStringList()) );
    QStringList PressureElemNames;
    PressureElemNames.append("0");
    fields.append( new UAVObjectField(QString("Pressure"), QString("kPa"), UAVObjectField::FLOAT32, PressureElemNames, QStringList()) );

    // Initialize object
    initializeFields(fields, (quint8*)&data, NUMBYTES);
    // Set the default field values
    setDefaultFieldValues();
}

/**
 * Get the default metadata for this object
 */
UAVObject::Metadata AltitudeActual::getDefaultMetadata()
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
void AltitudeActual::setDefaultFieldValues()
{

}

/**
 * Get the object data fields
 */
AltitudeActual::DataFields AltitudeActual::getData()
{
    QMutexLocker locker(mutex);
    return data;
}

/**
 * Set the object data fields
 */
void AltitudeActual::setData(const DataFields& data)
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
UAVDataObject* AltitudeActual::clone(quint32 instID)
{
    AltitudeActual* obj = new AltitudeActual();
    obj->initialize(instID, this->getMetaObject());
    return obj;
}

/**
 * Static function to retrieve an instance of the object.
 */
AltitudeActual* AltitudeActual::GetInstance(UAVObjectManager* objMngr, quint32 instID)
{
    return dynamic_cast<AltitudeActual*>(objMngr->getObject(AltitudeActual::OBJID, instID));
}
