/**
 ******************************************************************************
 *
 * @file       gpssatellites.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectsPlugin UAVObjects Plugin
 * @{
 *   
 * @note       Object definition file: gpssatellites.xml. 
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
#include "gpssatellites.h"
#include "uavobjectfield.h"

const QString GPSSatellites::NAME = QString("GPSSatellites");

/**
 * Constructor
 */
GPSSatellites::GPSSatellites(): UAVDataObject(OBJID, ISSINGLEINST, ISSETTINGS, NAME)
{
    // Create fields
    QList<UAVObjectField*> fields;
    QStringList SatsInViewElemNames;
    SatsInViewElemNames.append("0");
    fields.append( new UAVObjectField(QString("SatsInView"), QString(""), UAVObjectField::INT8, SatsInViewElemNames, QStringList()) );
    QStringList PRNElemNames;
    PRNElemNames.append("0");
    PRNElemNames.append("1");
    PRNElemNames.append("2");
    PRNElemNames.append("3");
    PRNElemNames.append("4");
    PRNElemNames.append("5");
    PRNElemNames.append("6");
    PRNElemNames.append("7");
    PRNElemNames.append("8");
    PRNElemNames.append("9");
    PRNElemNames.append("10");
    PRNElemNames.append("11");
    PRNElemNames.append("12");
    PRNElemNames.append("13");
    PRNElemNames.append("14");
    PRNElemNames.append("15");
    fields.append( new UAVObjectField(QString("PRN"), QString(""), UAVObjectField::INT8, PRNElemNames, QStringList()) );
    QStringList ElevationElemNames;
    ElevationElemNames.append("0");
    ElevationElemNames.append("1");
    ElevationElemNames.append("2");
    ElevationElemNames.append("3");
    ElevationElemNames.append("4");
    ElevationElemNames.append("5");
    ElevationElemNames.append("6");
    ElevationElemNames.append("7");
    ElevationElemNames.append("8");
    ElevationElemNames.append("9");
    ElevationElemNames.append("10");
    ElevationElemNames.append("11");
    ElevationElemNames.append("12");
    ElevationElemNames.append("13");
    ElevationElemNames.append("14");
    ElevationElemNames.append("15");
    fields.append( new UAVObjectField(QString("Elevation"), QString("degrees"), UAVObjectField::FLOAT32, ElevationElemNames, QStringList()) );
    QStringList AzimuthElemNames;
    AzimuthElemNames.append("0");
    AzimuthElemNames.append("1");
    AzimuthElemNames.append("2");
    AzimuthElemNames.append("3");
    AzimuthElemNames.append("4");
    AzimuthElemNames.append("5");
    AzimuthElemNames.append("6");
    AzimuthElemNames.append("7");
    AzimuthElemNames.append("8");
    AzimuthElemNames.append("9");
    AzimuthElemNames.append("10");
    AzimuthElemNames.append("11");
    AzimuthElemNames.append("12");
    AzimuthElemNames.append("13");
    AzimuthElemNames.append("14");
    AzimuthElemNames.append("15");
    fields.append( new UAVObjectField(QString("Azimuth"), QString("degrees"), UAVObjectField::FLOAT32, AzimuthElemNames, QStringList()) );
    QStringList SNRElemNames;
    SNRElemNames.append("0");
    SNRElemNames.append("1");
    SNRElemNames.append("2");
    SNRElemNames.append("3");
    SNRElemNames.append("4");
    SNRElemNames.append("5");
    SNRElemNames.append("6");
    SNRElemNames.append("7");
    SNRElemNames.append("8");
    SNRElemNames.append("9");
    SNRElemNames.append("10");
    SNRElemNames.append("11");
    SNRElemNames.append("12");
    SNRElemNames.append("13");
    SNRElemNames.append("14");
    SNRElemNames.append("15");
    fields.append( new UAVObjectField(QString("SNR"), QString(""), UAVObjectField::INT8, SNRElemNames, QStringList()) );

    // Initialize object
    initializeFields(fields, (quint8*)&data, NUMBYTES);
    // Set the default field values
    setDefaultFieldValues();
}

/**
 * Get the default metadata for this object
 */
UAVObject::Metadata GPSSatellites::getDefaultMetadata()
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
void GPSSatellites::setDefaultFieldValues()
{

}

/**
 * Get the object data fields
 */
GPSSatellites::DataFields GPSSatellites::getData()
{
    QMutexLocker locker(mutex);
    return data;
}

/**
 * Set the object data fields
 */
void GPSSatellites::setData(const DataFields& data)
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
UAVDataObject* GPSSatellites::clone(quint32 instID)
{
    GPSSatellites* obj = new GPSSatellites();
    obj->initialize(instID, this->getMetaObject());
    return obj;
}

/**
 * Static function to retrieve an instance of the object.
 */
GPSSatellites* GPSSatellites::GetInstance(UAVObjectManager* objMngr, quint32 instID)
{
    return dynamic_cast<GPSSatellites*>(objMngr->getObject(GPSSatellites::OBJID, instID));
}
