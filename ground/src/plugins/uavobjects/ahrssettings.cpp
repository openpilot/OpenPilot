/**
 ******************************************************************************
 *
 * @file       ahrssettings.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectsPlugin UAVObjects Plugin
 * @{
 *   
 * @note       Object definition file: ahrssettings.xml. 
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
#include "ahrssettings.h"
#include "uavobjectfield.h"

const QString AHRSSettings::NAME = QString("AHRSSettings");

/**
 * Constructor
 */
AHRSSettings::AHRSSettings(): UAVDataObject(OBJID, ISSINGLEINST, ISSETTINGS, NAME)
{
    // Create fields
    QList<UAVObjectField*> fields;
    QStringList AlgorithmElemNames;
    AlgorithmElemNames.append("0");
    QStringList AlgorithmEnumOptions;
    AlgorithmEnumOptions.append("SIMPLE");
    AlgorithmEnumOptions.append("INSGPS");
    fields.append( new UAVObjectField(QString("Algorithm"), QString(""), UAVObjectField::ENUM, AlgorithmElemNames, AlgorithmEnumOptions) );
    QStringList UpdateRawElemNames;
    UpdateRawElemNames.append("0");
    QStringList UpdateRawEnumOptions;
    UpdateRawEnumOptions.append("FALSE");
    UpdateRawEnumOptions.append("TRUE");
    fields.append( new UAVObjectField(QString("UpdateRaw"), QString("raw"), UAVObjectField::ENUM, UpdateRawElemNames, UpdateRawEnumOptions) );
    QStringList UpdateFilteredElemNames;
    UpdateFilteredElemNames.append("0");
    QStringList UpdateFilteredEnumOptions;
    UpdateFilteredEnumOptions.append("FALSE");
    UpdateFilteredEnumOptions.append("TRUE");
    fields.append( new UAVObjectField(QString("UpdateFiltered"), QString("raw"), UAVObjectField::ENUM, UpdateFilteredElemNames, UpdateFilteredEnumOptions) );
    QStringList UpdatePeriodElemNames;
    UpdatePeriodElemNames.append("0");
    fields.append( new UAVObjectField(QString("UpdatePeriod"), QString("ms"), UAVObjectField::INT32, UpdatePeriodElemNames, QStringList()) );

    // Initialize object
    initializeFields(fields, (quint8*)&data, NUMBYTES);
    // Set the default field values
    setDefaultFieldValues();
}

/**
 * Get the default metadata for this object
 */
UAVObject::Metadata AHRSSettings::getDefaultMetadata()
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
void AHRSSettings::setDefaultFieldValues()
{
    data.Algorithm = 1;
    data.UpdateRaw = 0;
    data.UpdateFiltered = 1;
    data.UpdatePeriod = 20;

}

/**
 * Get the object data fields
 */
AHRSSettings::DataFields AHRSSettings::getData()
{
    QMutexLocker locker(mutex);
    return data;
}

/**
 * Set the object data fields
 */
void AHRSSettings::setData(const DataFields& data)
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
UAVDataObject* AHRSSettings::clone(quint32 instID)
{
    AHRSSettings* obj = new AHRSSettings();
    obj->initialize(instID, this->getMetaObject());
    return obj;
}

/**
 * Static function to retrieve an instance of the object.
 */
AHRSSettings* AHRSSettings::GetInstance(UAVObjectManager* objMngr, quint32 instID)
{
    return dynamic_cast<AHRSSettings*>(objMngr->getObject(AHRSSettings::OBJID, instID));
}
