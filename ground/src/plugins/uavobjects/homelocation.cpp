/**
 ******************************************************************************
 *
 * @file       homelocation.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectsPlugin UAVObjects Plugin
 * @{
 *   
 * @note       Object definition file: homelocation.xml. 
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
#include "homelocation.h"
#include "uavobjectfield.h"

const QString HomeLocation::NAME = QString("HomeLocation");

/**
 * Constructor
 */
HomeLocation::HomeLocation(): UAVDataObject(OBJID, ISSINGLEINST, ISSETTINGS, NAME)
{
    // Create fields
    QList<UAVObjectField*> fields;
    QStringList SetElemNames;
    SetElemNames.append("0");
    QStringList SetEnumOptions;
    SetEnumOptions.append("FALSE");
    SetEnumOptions.append("TRUE");
    fields.append( new UAVObjectField(QString("Set"), QString(""), UAVObjectField::ENUM, SetElemNames, SetEnumOptions) );
    QStringList LatitudeElemNames;
    LatitudeElemNames.append("0");
    fields.append( new UAVObjectField(QString("Latitude"), QString("deg * 10e6"), UAVObjectField::INT32, LatitudeElemNames, QStringList()) );
    QStringList LongitudeElemNames;
    LongitudeElemNames.append("0");
    fields.append( new UAVObjectField(QString("Longitude"), QString("deg * 10e6"), UAVObjectField::INT32, LongitudeElemNames, QStringList()) );
    QStringList AltitudeElemNames;
    AltitudeElemNames.append("0");
    fields.append( new UAVObjectField(QString("Altitude"), QString("m over geoid"), UAVObjectField::FLOAT32, AltitudeElemNames, QStringList()) );
    QStringList ECEFElemNames;
    ECEFElemNames.append("0");
    ECEFElemNames.append("1");
    ECEFElemNames.append("2");
    fields.append( new UAVObjectField(QString("ECEF"), QString("m"), UAVObjectField::FLOAT32, ECEFElemNames, QStringList()) );
    QStringList RNEElemNames;
    RNEElemNames.append("0");
    RNEElemNames.append("1");
    RNEElemNames.append("2");
    RNEElemNames.append("3");
    RNEElemNames.append("4");
    RNEElemNames.append("5");
    RNEElemNames.append("6");
    RNEElemNames.append("7");
    RNEElemNames.append("8");
    fields.append( new UAVObjectField(QString("RNE"), QString(""), UAVObjectField::FLOAT32, RNEElemNames, QStringList()) );
    QStringList BeElemNames;
    BeElemNames.append("0");
    BeElemNames.append("1");
    BeElemNames.append("2");
    fields.append( new UAVObjectField(QString("Be"), QString(""), UAVObjectField::FLOAT32, BeElemNames, QStringList()) );

    // Initialize object
    initializeFields(fields, (quint8*)&data, NUMBYTES);
    // Set the default field values
    setDefaultFieldValues();
}

/**
 * Get the default metadata for this object
 */
UAVObject::Metadata HomeLocation::getDefaultMetadata()
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
void HomeLocation::setDefaultFieldValues()
{
    data.Set = 0;
    data.Latitude = 0;
    data.Longitude = 0;
    data.Altitude = 0;
    data.ECEF[0] = 0;
    data.ECEF[1] = 0;
    data.ECEF[2] = 0;
    data.RNE[0] = 0;
    data.RNE[1] = 0;
    data.RNE[2] = 0;
    data.RNE[3] = 0;
    data.RNE[4] = 0;
    data.RNE[5] = 0;
    data.RNE[6] = 0;
    data.RNE[7] = 0;
    data.RNE[8] = 0;
    data.Be[0] = 0;
    data.Be[1] = 0;
    data.Be[2] = 0;

}

/**
 * Get the object data fields
 */
HomeLocation::DataFields HomeLocation::getData()
{
    QMutexLocker locker(mutex);
    return data;
}

/**
 * Set the object data fields
 */
void HomeLocation::setData(const DataFields& data)
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
UAVDataObject* HomeLocation::clone(quint32 instID)
{
    HomeLocation* obj = new HomeLocation();
    obj->initialize(instID, this->getMetaObject());
    return obj;
}

/**
 * Static function to retrieve an instance of the object.
 */
HomeLocation* HomeLocation::GetInstance(UAVObjectManager* objMngr, quint32 instID)
{
    return dynamic_cast<HomeLocation*>(objMngr->getObject(HomeLocation::OBJID, instID));
}
