/**
 ******************************************************************************
 *
 * @file       stabilizationsettings.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectsPlugin UAVObjects Plugin
 * @{
 *   
 * @note       Object definition file: stabilizationsettings.xml. 
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
#include "stabilizationsettings.h"
#include "uavobjectfield.h"

const QString StabilizationSettings::NAME = QString("StabilizationSettings");

/**
 * Constructor
 */
StabilizationSettings::StabilizationSettings(): UAVDataObject(OBJID, ISSINGLEINST, ISSETTINGS, NAME)
{
    // Create fields
    QList<UAVObjectField*> fields;
    QStringList RollMaxElemNames;
    RollMaxElemNames.append("0");
    fields.append( new UAVObjectField(QString("RollMax"), QString("degrees"), UAVObjectField::UINT8, RollMaxElemNames, QStringList()) );
    QStringList PitchMaxElemNames;
    PitchMaxElemNames.append("0");
    fields.append( new UAVObjectField(QString("PitchMax"), QString("degrees"), UAVObjectField::UINT8, PitchMaxElemNames, QStringList()) );
    QStringList YawMaxElemNames;
    YawMaxElemNames.append("0");
    fields.append( new UAVObjectField(QString("YawMax"), QString("degrees"), UAVObjectField::UINT8, YawMaxElemNames, QStringList()) );
    QStringList YawModeElemNames;
    YawModeElemNames.append("0");
    QStringList YawModeEnumOptions;
    YawModeEnumOptions.append("rate");
    YawModeEnumOptions.append("heading");
    fields.append( new UAVObjectField(QString("YawMode"), QString("degrees"), UAVObjectField::ENUM, YawModeElemNames, YawModeEnumOptions) );
    QStringList ThrottleMaxElemNames;
    ThrottleMaxElemNames.append("0");
    fields.append( new UAVObjectField(QString("ThrottleMax"), QString("frac"), UAVObjectField::FLOAT32, ThrottleMaxElemNames, QStringList()) );
    QStringList ThrottleMinElemNames;
    ThrottleMinElemNames.append("0");
    fields.append( new UAVObjectField(QString("ThrottleMin"), QString("frac"), UAVObjectField::FLOAT32, ThrottleMinElemNames, QStringList()) );
    QStringList RollIntegralLimitElemNames;
    RollIntegralLimitElemNames.append("0");
    fields.append( new UAVObjectField(QString("RollIntegralLimit"), QString(""), UAVObjectField::FLOAT32, RollIntegralLimitElemNames, QStringList()) );
    QStringList PitchIntegralLimitElemNames;
    PitchIntegralLimitElemNames.append("0");
    fields.append( new UAVObjectField(QString("PitchIntegralLimit"), QString(""), UAVObjectField::FLOAT32, PitchIntegralLimitElemNames, QStringList()) );
    QStringList YawIntegralLimitElemNames;
    YawIntegralLimitElemNames.append("0");
    fields.append( new UAVObjectField(QString("YawIntegralLimit"), QString(""), UAVObjectField::FLOAT32, YawIntegralLimitElemNames, QStringList()) );
    QStringList PitchKpElemNames;
    PitchKpElemNames.append("0");
    fields.append( new UAVObjectField(QString("PitchKp"), QString(""), UAVObjectField::FLOAT32, PitchKpElemNames, QStringList()) );
    QStringList PitchKiElemNames;
    PitchKiElemNames.append("0");
    fields.append( new UAVObjectField(QString("PitchKi"), QString(""), UAVObjectField::FLOAT32, PitchKiElemNames, QStringList()) );
    QStringList PitchKdElemNames;
    PitchKdElemNames.append("0");
    fields.append( new UAVObjectField(QString("PitchKd"), QString(""), UAVObjectField::FLOAT32, PitchKdElemNames, QStringList()) );
    QStringList RollKpElemNames;
    RollKpElemNames.append("0");
    fields.append( new UAVObjectField(QString("RollKp"), QString(""), UAVObjectField::FLOAT32, RollKpElemNames, QStringList()) );
    QStringList RollKiElemNames;
    RollKiElemNames.append("0");
    fields.append( new UAVObjectField(QString("RollKi"), QString(""), UAVObjectField::FLOAT32, RollKiElemNames, QStringList()) );
    QStringList RollKdElemNames;
    RollKdElemNames.append("0");
    fields.append( new UAVObjectField(QString("RollKd"), QString(""), UAVObjectField::FLOAT32, RollKdElemNames, QStringList()) );
    QStringList YawKpElemNames;
    YawKpElemNames.append("0");
    fields.append( new UAVObjectField(QString("YawKp"), QString(""), UAVObjectField::FLOAT32, YawKpElemNames, QStringList()) );
    QStringList YawKiElemNames;
    YawKiElemNames.append("0");
    fields.append( new UAVObjectField(QString("YawKi"), QString(""), UAVObjectField::FLOAT32, YawKiElemNames, QStringList()) );
    QStringList YawKdElemNames;
    YawKdElemNames.append("0");
    fields.append( new UAVObjectField(QString("YawKd"), QString(""), UAVObjectField::FLOAT32, YawKdElemNames, QStringList()) );

    // Initialize object
    initializeFields(fields, (quint8*)&data, NUMBYTES);
    // Set the default field values
    setDefaultFieldValues();
}

/**
 * Get the default metadata for this object
 */
UAVObject::Metadata StabilizationSettings::getDefaultMetadata()
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
void StabilizationSettings::setDefaultFieldValues()
{
    data.RollMax = 35;
    data.PitchMax = 35;
    data.YawMax = 35;
    data.YawMode = 0;
    data.ThrottleMax = 1;
    data.ThrottleMin = -1;
    data.RollIntegralLimit = 0.5;
    data.PitchIntegralLimit = 0.5;
    data.YawIntegralLimit = 0.5;
    data.PitchKp = 0.04;
    data.PitchKi = 4e-06;
    data.PitchKd = 0.01;
    data.RollKp = 0.02;
    data.RollKi = 4e-06;
    data.RollKd = 0.01;
    data.YawKp = 0.04;
    data.YawKi = 4e-06;
    data.YawKd = 0.01;

}

/**
 * Get the object data fields
 */
StabilizationSettings::DataFields StabilizationSettings::getData()
{
    QMutexLocker locker(mutex);
    return data;
}

/**
 * Set the object data fields
 */
void StabilizationSettings::setData(const DataFields& data)
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
UAVDataObject* StabilizationSettings::clone(quint32 instID)
{
    StabilizationSettings* obj = new StabilizationSettings();
    obj->initialize(instID, this->getMetaObject());
    return obj;
}

/**
 * Static function to retrieve an instance of the object.
 */
StabilizationSettings* StabilizationSettings::GetInstance(UAVObjectManager* objMngr, quint32 instID)
{
    return dynamic_cast<StabilizationSettings*>(objMngr->getObject(StabilizationSettings::OBJID, instID));
}
