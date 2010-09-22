/**
 ******************************************************************************
 *
 * @file       guidancesettings.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectsPlugin UAVObjects Plugin
 * @{
 *   
 * @note       Object definition file: guidancesettings.xml. 
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
#include "guidancesettings.h"
#include "uavobjectfield.h"

const QString GuidanceSettings::NAME = QString("GuidanceSettings");

/**
 * Constructor
 */
GuidanceSettings::GuidanceSettings(): UAVDataObject(OBJID, ISSINGLEINST, ISSETTINGS, NAME)
{
    // Create fields
    QList<UAVObjectField*> fields;
    QStringList UpdatePeriodElemNames;
    UpdatePeriodElemNames.append("0");
    fields.append( new UAVObjectField(QString("UpdatePeriod"), QString("ms"), UAVObjectField::UINT16, UpdatePeriodElemNames, QStringList()) );
    QStringList RollMaxElemNames;
    RollMaxElemNames.append("0");
    fields.append( new UAVObjectField(QString("RollMax"), QString("degrees"), UAVObjectField::FLOAT32, RollMaxElemNames, QStringList()) );
    QStringList PitchMaxElemNames;
    PitchMaxElemNames.append("0");
    fields.append( new UAVObjectField(QString("PitchMax"), QString("degrees"), UAVObjectField::FLOAT32, PitchMaxElemNames, QStringList()) );
    QStringList PitchMinElemNames;
    PitchMinElemNames.append("0");
    fields.append( new UAVObjectField(QString("PitchMin"), QString("degrees"), UAVObjectField::FLOAT32, PitchMinElemNames, QStringList()) );
    QStringList PitchRollEpsilonElemNames;
    PitchRollEpsilonElemNames.append("0");
    fields.append( new UAVObjectField(QString("PitchRollEpsilon"), QString("degrees"), UAVObjectField::FLOAT32, PitchRollEpsilonElemNames, QStringList()) );
    QStringList ThrottleMaxElemNames;
    ThrottleMaxElemNames.append("0");
    fields.append( new UAVObjectField(QString("ThrottleMax"), QString("%"), UAVObjectField::FLOAT32, ThrottleMaxElemNames, QStringList()) );
    QStringList ThrottleMinElemNames;
    ThrottleMinElemNames.append("0");
    fields.append( new UAVObjectField(QString("ThrottleMin"), QString("%"), UAVObjectField::FLOAT32, ThrottleMinElemNames, QStringList()) );
    QStringList SpeedMaxElemNames;
    SpeedMaxElemNames.append("0");
    fields.append( new UAVObjectField(QString("SpeedMax"), QString("m/s"), UAVObjectField::FLOAT32, SpeedMaxElemNames, QStringList()) );
    QStringList SpeedMinElemNames;
    SpeedMinElemNames.append("0");
    fields.append( new UAVObjectField(QString("SpeedMin"), QString("m/s"), UAVObjectField::FLOAT32, SpeedMinElemNames, QStringList()) );
    QStringList SpeedKpElemNames;
    SpeedKpElemNames.append("0");
    fields.append( new UAVObjectField(QString("SpeedKp"), QString(""), UAVObjectField::FLOAT32, SpeedKpElemNames, QStringList()) );
    QStringList SpeedKiElemNames;
    SpeedKiElemNames.append("0");
    fields.append( new UAVObjectField(QString("SpeedKi"), QString(""), UAVObjectField::FLOAT32, SpeedKiElemNames, QStringList()) );
    QStringList SpeedKdElemNames;
    SpeedKdElemNames.append("0");
    fields.append( new UAVObjectField(QString("SpeedKd"), QString(""), UAVObjectField::FLOAT32, SpeedKdElemNames, QStringList()) );
    QStringList EnergyKpElemNames;
    EnergyKpElemNames.append("0");
    fields.append( new UAVObjectField(QString("EnergyKp"), QString(""), UAVObjectField::FLOAT32, EnergyKpElemNames, QStringList()) );
    QStringList EnergyKiElemNames;
    EnergyKiElemNames.append("0");
    fields.append( new UAVObjectField(QString("EnergyKi"), QString(""), UAVObjectField::FLOAT32, EnergyKiElemNames, QStringList()) );
    QStringList EnergyKdElemNames;
    EnergyKdElemNames.append("0");
    fields.append( new UAVObjectField(QString("EnergyKd"), QString(""), UAVObjectField::FLOAT32, EnergyKdElemNames, QStringList()) );
    QStringList LateralKpElemNames;
    LateralKpElemNames.append("0");
    fields.append( new UAVObjectField(QString("LateralKp"), QString(""), UAVObjectField::FLOAT32, LateralKpElemNames, QStringList()) );
    QStringList LateralKiElemNames;
    LateralKiElemNames.append("0");
    fields.append( new UAVObjectField(QString("LateralKi"), QString(""), UAVObjectField::FLOAT32, LateralKiElemNames, QStringList()) );
    QStringList LateralKdElemNames;
    LateralKdElemNames.append("0");
    fields.append( new UAVObjectField(QString("LateralKd"), QString(""), UAVObjectField::FLOAT32, LateralKdElemNames, QStringList()) );
    QStringList CourseKpElemNames;
    CourseKpElemNames.append("0");
    fields.append( new UAVObjectField(QString("CourseKp"), QString(""), UAVObjectField::FLOAT32, CourseKpElemNames, QStringList()) );
    QStringList CourseKiElemNames;
    CourseKiElemNames.append("0");
    fields.append( new UAVObjectField(QString("CourseKi"), QString(""), UAVObjectField::FLOAT32, CourseKiElemNames, QStringList()) );
    QStringList CourseKdElemNames;
    CourseKdElemNames.append("0");
    fields.append( new UAVObjectField(QString("CourseKd"), QString(""), UAVObjectField::FLOAT32, CourseKdElemNames, QStringList()) );

    // Initialize object
    initializeFields(fields, (quint8*)&data, NUMBYTES);
    // Set the default field values
    setDefaultFieldValues();
}

/**
 * Get the default metadata for this object
 */
UAVObject::Metadata GuidanceSettings::getDefaultMetadata()
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
void GuidanceSettings::setDefaultFieldValues()
{
    data.UpdatePeriod = 10;
    data.RollMax = 35;
    data.PitchMax = 35;
    data.PitchMin = -35;
    data.PitchRollEpsilon = 10;
    data.ThrottleMax = 1;
    data.ThrottleMin = 0;
    data.SpeedMax = 100;
    data.SpeedMin = 10;
    data.SpeedKp = 0.04;
    data.SpeedKi = 4e-06;
    data.SpeedKd = 0.01;
    data.EnergyKp = 0.04;
    data.EnergyKi = 4e-06;
    data.EnergyKd = 0.01;
    data.LateralKp = 0.04;
    data.LateralKi = 4e-06;
    data.LateralKd = 0.01;
    data.CourseKp = 0.04;
    data.CourseKi = 4e-06;
    data.CourseKd = 0.01;

}

/**
 * Get the object data fields
 */
GuidanceSettings::DataFields GuidanceSettings::getData()
{
    QMutexLocker locker(mutex);
    return data;
}

/**
 * Set the object data fields
 */
void GuidanceSettings::setData(const DataFields& data)
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
UAVDataObject* GuidanceSettings::clone(quint32 instID)
{
    GuidanceSettings* obj = new GuidanceSettings();
    obj->initialize(instID, this->getMetaObject());
    return obj;
}

/**
 * Static function to retrieve an instance of the object.
 */
GuidanceSettings* GuidanceSettings::GetInstance(UAVObjectManager* objMngr, quint32 instID)
{
    return dynamic_cast<GuidanceSettings*>(objMngr->getObject(GuidanceSettings::OBJID, instID));
}
