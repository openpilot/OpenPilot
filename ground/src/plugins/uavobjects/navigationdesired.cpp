/**
 ******************************************************************************
 *
 * @file       navigationdesired.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectsPlugin UAVObjects Plugin
 * @{
 *   
 * @note       Object definition file: navigationdesired.xml. 
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
#include "navigationdesired.h"
#include "uavobjectfield.h"

const QString NavigationDesired::NAME = QString("NavigationDesired");

/**
 * Constructor
 */
NavigationDesired::NavigationDesired(): UAVDataObject(OBJID, ISSINGLEINST, ISSETTINGS, NAME)
{
    // Create fields
    QList<UAVObjectField*> fields;
    QStringList LatitudeElemNames;
    LatitudeElemNames.append("0");
    fields.append( new UAVObjectField(QString("Latitude"), QString("degrees"), UAVObjectField::FLOAT32, LatitudeElemNames, QStringList()) );
    QStringList LongitudeElemNames;
    LongitudeElemNames.append("0");
    fields.append( new UAVObjectField(QString("Longitude"), QString("degrees"), UAVObjectField::FLOAT32, LongitudeElemNames, QStringList()) );
    QStringList AltitudeElemNames;
    AltitudeElemNames.append("0");
    fields.append( new UAVObjectField(QString("Altitude"), QString("meters"), UAVObjectField::FLOAT32, AltitudeElemNames, QStringList()) );
    QStringList SpeedElemNames;
    SpeedElemNames.append("0");
    fields.append( new UAVObjectField(QString("Speed"), QString("m/s"), UAVObjectField::FLOAT32, SpeedElemNames, QStringList()) );

    // Initialize object
    initializeFields(fields, (quint8*)&data, NUMBYTES);
    // Set the default field values
    setDefaultFieldValues();
}

/**
 * Get the default metadata for this object
 */
UAVObject::Metadata NavigationDesired::getDefaultMetadata()
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
void NavigationDesired::setDefaultFieldValues()
{

}

/**
 * Get the object data fields
 */
NavigationDesired::DataFields NavigationDesired::getData()
{
    QMutexLocker locker(mutex);
    return data;
}

/**
 * Set the object data fields
 */
void NavigationDesired::setData(const DataFields& data)
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
UAVDataObject* NavigationDesired::clone(quint32 instID)
{
    NavigationDesired* obj = new NavigationDesired();
    obj->initialize(instID, this->getMetaObject());
    return obj;
}

/**
 * Static function to retrieve an instance of the object.
 */
NavigationDesired* NavigationDesired::GetInstance(UAVObjectManager* objMngr, quint32 instID)
{
    return dynamic_cast<NavigationDesired*>(objMngr->getObject(NavigationDesired::OBJID, instID));
}
