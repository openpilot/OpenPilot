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
const QString StabilizationSettings::DESCRIPTION = QString("PID settings used by the Stabilization module to combine the @ref AttitudeActual and @ref AttitudeDesired to compute @ref ActuatorDesired");

/**
 * Constructor
 */
StabilizationSettings::StabilizationSettings(): UAVDataObject(OBJID, ISSINGLEINST, ISSETTINGS, NAME)
{
    // Create fields
    QList<UAVObjectField*> fields;
    QStringList RollMaxElemNames;
    RollMaxElemNames.append("0");
    fields.append( new UAVObjectField(QString("RollMax"), QString("Degrees"), UAVObjectField::UINT8, RollMaxElemNames, QStringList()) );
    QStringList PitchMaxElemNames;
    PitchMaxElemNames.append("0");
    fields.append( new UAVObjectField(QString("PitchMax"), QString("Degrees"), UAVObjectField::UINT8, PitchMaxElemNames, QStringList()) );
    QStringList YawMaxElemNames;
    YawMaxElemNames.append("0");
    fields.append( new UAVObjectField(QString("YawMax"), QString("Degrees"), UAVObjectField::UINT8, YawMaxElemNames, QStringList()) );
    QStringList ManualRateElemNames;
    ManualRateElemNames.append("Roll");
    ManualRateElemNames.append("Pitch");
    ManualRateElemNames.append("Yaw");
    fields.append( new UAVObjectField(QString("ManualRate"), QString("Degrees/sec"), UAVObjectField::FLOAT32, ManualRateElemNames, QStringList()) );
    QStringList MaximumRateElemNames;
    MaximumRateElemNames.append("Roll");
    MaximumRateElemNames.append("Pitch");
    MaximumRateElemNames.append("Yaw");
    fields.append( new UAVObjectField(QString("MaximumRate"), QString("Degrees/sec"), UAVObjectField::FLOAT32, MaximumRateElemNames, QStringList()) );
    QStringList RollRatePIElemNames;
    RollRatePIElemNames.append("Kp");
    RollRatePIElemNames.append("Ki");
    RollRatePIElemNames.append("ILimit");
    fields.append( new UAVObjectField(QString("RollRatePI"), QString(""), UAVObjectField::FLOAT32, RollRatePIElemNames, QStringList()) );
    QStringList PitchRatePIElemNames;
    PitchRatePIElemNames.append("Kp");
    PitchRatePIElemNames.append("Ki");
    PitchRatePIElemNames.append("ILimit");
    fields.append( new UAVObjectField(QString("PitchRatePI"), QString(""), UAVObjectField::FLOAT32, PitchRatePIElemNames, QStringList()) );
    QStringList YawRatePIElemNames;
    YawRatePIElemNames.append("Kp");
    YawRatePIElemNames.append("Ki");
    YawRatePIElemNames.append("ILimit");
    fields.append( new UAVObjectField(QString("YawRatePI"), QString(""), UAVObjectField::FLOAT32, YawRatePIElemNames, QStringList()) );
    QStringList RollPIElemNames;
    RollPIElemNames.append("Kp");
    RollPIElemNames.append("Ki");
    RollPIElemNames.append("ILimit");
    fields.append( new UAVObjectField(QString("RollPI"), QString(""), UAVObjectField::FLOAT32, RollPIElemNames, QStringList()) );
    QStringList PitchPIElemNames;
    PitchPIElemNames.append("Kp");
    PitchPIElemNames.append("Ki");
    PitchPIElemNames.append("ILimit");
    fields.append( new UAVObjectField(QString("PitchPI"), QString(""), UAVObjectField::FLOAT32, PitchPIElemNames, QStringList()) );
    QStringList YawPIElemNames;
    YawPIElemNames.append("Kp");
    YawPIElemNames.append("Ki");
    YawPIElemNames.append("ILimit");
    fields.append( new UAVObjectField(QString("YawPI"), QString(""), UAVObjectField::FLOAT32, YawPIElemNames, QStringList()) );

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
    data.ManualRate[0] = 150;
    data.ManualRate[1] = 150;
    data.ManualRate[2] = 150;
    data.MaximumRate[0] = 300;
    data.MaximumRate[1] = 300;
    data.MaximumRate[2] = 300;
    data.RollRatePI[0] = 0.0015;
    data.RollRatePI[1] = 0;
    data.RollRatePI[2] = 0.5;
    data.PitchRatePI[0] = 0.0015;
    data.PitchRatePI[1] = 0;
    data.PitchRatePI[2] = 0.5;
    data.YawRatePI[0] = 0.003;
    data.YawRatePI[1] = 0;
    data.YawRatePI[2] = 0.5;
    data.RollPI[0] = 2;
    data.RollPI[1] = 0;
    data.RollPI[2] = 0.5;
    data.PitchPI[0] = 2;
    data.PitchPI[1] = 0;
    data.PitchPI[2] = 0.5;
    data.YawPI[0] = 2;
    data.YawPI[1] = 0;
    data.YawPI[2] = 0.5;

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
