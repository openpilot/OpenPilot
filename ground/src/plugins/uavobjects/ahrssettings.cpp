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
    AlgorithmEnumOptions.append("INSGPS_INDOOR_NOMAG");
    AlgorithmEnumOptions.append("INSGPS_INDOOR");
    AlgorithmEnumOptions.append("INSGPS_OUTDOOR");
    fields.append( new UAVObjectField(QString("Algorithm"), QString(""), UAVObjectField::ENUM, AlgorithmElemNames, AlgorithmEnumOptions) );
    QStringList DownsamplingElemNames;
    DownsamplingElemNames.append("0");
    fields.append( new UAVObjectField(QString("Downsampling"), QString(""), UAVObjectField::UINT8, DownsamplingElemNames, QStringList()) );
    QStringList UpdatePeriodElemNames;
    UpdatePeriodElemNames.append("0");
    fields.append( new UAVObjectField(QString("UpdatePeriod"), QString("ms"), UAVObjectField::UINT8, UpdatePeriodElemNames, QStringList()) );
    QStringList BiasCorrectedRawElemNames;
    BiasCorrectedRawElemNames.append("0");
    QStringList BiasCorrectedRawEnumOptions;
    BiasCorrectedRawEnumOptions.append("TRUE");
    BiasCorrectedRawEnumOptions.append("FALSE");
    fields.append( new UAVObjectField(QString("BiasCorrectedRaw"), QString(""), UAVObjectField::ENUM, BiasCorrectedRawElemNames, BiasCorrectedRawEnumOptions) );
    QStringList YawBiasElemNames;
    YawBiasElemNames.append("0");
    fields.append( new UAVObjectField(QString("YawBias"), QString(""), UAVObjectField::FLOAT32, YawBiasElemNames, QStringList()) );
    QStringList PitchBiasElemNames;
    PitchBiasElemNames.append("0");
    fields.append( new UAVObjectField(QString("PitchBias"), QString(""), UAVObjectField::FLOAT32, PitchBiasElemNames, QStringList()) );
    QStringList RollBiasElemNames;
    RollBiasElemNames.append("0");
    fields.append( new UAVObjectField(QString("RollBias"), QString(""), UAVObjectField::FLOAT32, RollBiasElemNames, QStringList()) );

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
    data.Downsampling = 20;
    data.UpdatePeriod = 1;
    data.BiasCorrectedRaw = 0;
    data.YawBias = 0;
    data.PitchBias = 0;
    data.RollBias = 0;

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
