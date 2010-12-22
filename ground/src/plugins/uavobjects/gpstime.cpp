/**
 ******************************************************************************
 *
 * @file       gpstime.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectsPlugin UAVObjects Plugin
 * @{
 *   
 * @note       Object definition file: gpstime.xml. 
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
#include "gpstime.h"
#include "uavobjectfield.h"

const QString GPSTime::NAME = QString("GPSTime");
const QString GPSTime::DESCRIPTION = QString("Contains the GPS time from @ref GPSModule.  Required to compute the world magnetic model correctly when setting the home location.");

/**
 * Constructor
 */
GPSTime::GPSTime(): UAVDataObject(OBJID, ISSINGLEINST, ISSETTINGS, NAME)
{
    // Create fields
    QList<UAVObjectField*> fields;
    QStringList MonthElemNames;
    MonthElemNames.append("0");
    fields.append( new UAVObjectField(QString("Month"), QString(""), UAVObjectField::INT8, MonthElemNames, QStringList()) );
    QStringList DayElemNames;
    DayElemNames.append("0");
    fields.append( new UAVObjectField(QString("Day"), QString(""), UAVObjectField::INT8, DayElemNames, QStringList()) );
    QStringList YearElemNames;
    YearElemNames.append("0");
    fields.append( new UAVObjectField(QString("Year"), QString(""), UAVObjectField::INT16, YearElemNames, QStringList()) );
    QStringList HourElemNames;
    HourElemNames.append("0");
    fields.append( new UAVObjectField(QString("Hour"), QString(""), UAVObjectField::INT8, HourElemNames, QStringList()) );
    QStringList MinuteElemNames;
    MinuteElemNames.append("0");
    fields.append( new UAVObjectField(QString("Minute"), QString(""), UAVObjectField::INT8, MinuteElemNames, QStringList()) );
    QStringList SecondElemNames;
    SecondElemNames.append("0");
    fields.append( new UAVObjectField(QString("Second"), QString(""), UAVObjectField::INT8, SecondElemNames, QStringList()) );

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
UAVObject::Metadata GPSTime::getDefaultMetadata()
{
    UAVObject::Metadata metadata;
    metadata.flightAccess = ACCESS_READWRITE;
    metadata.gcsAccess = ACCESS_READWRITE;
    metadata.gcsTelemetryAcked = 0;
    metadata.gcsTelemetryUpdateMode = UAVObject::UPDATEMODE_MANUAL;
    metadata.gcsTelemetryUpdatePeriod = 0;
    metadata.flightTelemetryAcked = 0;
    metadata.flightTelemetryUpdateMode = UAVObject::UPDATEMODE_PERIODIC;
    metadata.flightTelemetryUpdatePeriod = 10000;
    metadata.loggingUpdateMode = UAVObject::UPDATEMODE_PERIODIC;
    metadata.loggingUpdatePeriod = 30000;
    return metadata;
}

/**
 * Initialize object fields with the default values.
 * If a default value is not specified the object fields
 * will be initialized to zero.
 */
void GPSTime::setDefaultFieldValues()
{

}

/**
 * Get the object data fields
 */
GPSTime::DataFields GPSTime::getData()
{
    QMutexLocker locker(mutex);
    return data;
}

/**
 * Set the object data fields
 */
void GPSTime::setData(const DataFields& data)
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
UAVDataObject* GPSTime::clone(quint32 instID)
{
    GPSTime* obj = new GPSTime();
    obj->initialize(instID, this->getMetaObject());
    return obj;
}

/**
 * Static function to retrieve an instance of the object.
 */
GPSTime* GPSTime::GetInstance(UAVObjectManager* objMngr, quint32 instID)
{
    return dynamic_cast<GPSTime*>(objMngr->getObject(GPSTime::OBJID, instID));
}
