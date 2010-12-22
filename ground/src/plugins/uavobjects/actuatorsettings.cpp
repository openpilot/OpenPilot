/**
 ******************************************************************************
 *
 * @file       actuatorsettings.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectsPlugin UAVObjects Plugin
 * @{
 *   
 * @note       Object definition file: actuatorsettings.xml. 
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
#include "actuatorsettings.h"
#include "uavobjectfield.h"

const QString ActuatorSettings::NAME = QString("ActuatorSettings");
const QString ActuatorSettings::DESCRIPTION = QString("Settings for the @ref ActuatorModule that controls the channel assignments for the mixer based on AircraftType");

/**
 * Constructor
 */
ActuatorSettings::ActuatorSettings(): UAVDataObject(OBJID, ISSINGLEINST, ISSETTINGS, NAME)
{
    // Create fields
    QList<UAVObjectField*> fields;
    QStringList FixedWingRoll1ElemNames;
    FixedWingRoll1ElemNames.append("0");
    QStringList FixedWingRoll1EnumOptions;
    FixedWingRoll1EnumOptions.append("Channel1");
    FixedWingRoll1EnumOptions.append("Channel2");
    FixedWingRoll1EnumOptions.append("Channel3");
    FixedWingRoll1EnumOptions.append("Channel4");
    FixedWingRoll1EnumOptions.append("Channel5");
    FixedWingRoll1EnumOptions.append("Channel6");
    FixedWingRoll1EnumOptions.append("Channel7");
    FixedWingRoll1EnumOptions.append("Channel8");
    FixedWingRoll1EnumOptions.append("None");
    fields.append( new UAVObjectField(QString("FixedWingRoll1"), QString("channel"), UAVObjectField::ENUM, FixedWingRoll1ElemNames, FixedWingRoll1EnumOptions) );
    QStringList FixedWingRoll2ElemNames;
    FixedWingRoll2ElemNames.append("0");
    QStringList FixedWingRoll2EnumOptions;
    FixedWingRoll2EnumOptions.append("Channel1");
    FixedWingRoll2EnumOptions.append("Channel2");
    FixedWingRoll2EnumOptions.append("Channel3");
    FixedWingRoll2EnumOptions.append("Channel4");
    FixedWingRoll2EnumOptions.append("Channel5");
    FixedWingRoll2EnumOptions.append("Channel6");
    FixedWingRoll2EnumOptions.append("Channel7");
    FixedWingRoll2EnumOptions.append("Channel8");
    FixedWingRoll2EnumOptions.append("None");
    fields.append( new UAVObjectField(QString("FixedWingRoll2"), QString("channel"), UAVObjectField::ENUM, FixedWingRoll2ElemNames, FixedWingRoll2EnumOptions) );
    QStringList FixedWingPitch1ElemNames;
    FixedWingPitch1ElemNames.append("0");
    QStringList FixedWingPitch1EnumOptions;
    FixedWingPitch1EnumOptions.append("Channel1");
    FixedWingPitch1EnumOptions.append("Channel2");
    FixedWingPitch1EnumOptions.append("Channel3");
    FixedWingPitch1EnumOptions.append("Channel4");
    FixedWingPitch1EnumOptions.append("Channel5");
    FixedWingPitch1EnumOptions.append("Channel6");
    FixedWingPitch1EnumOptions.append("Channel7");
    FixedWingPitch1EnumOptions.append("Channel8");
    FixedWingPitch1EnumOptions.append("None");
    fields.append( new UAVObjectField(QString("FixedWingPitch1"), QString("channel"), UAVObjectField::ENUM, FixedWingPitch1ElemNames, FixedWingPitch1EnumOptions) );
    QStringList FixedWingPitch2ElemNames;
    FixedWingPitch2ElemNames.append("0");
    QStringList FixedWingPitch2EnumOptions;
    FixedWingPitch2EnumOptions.append("Channel1");
    FixedWingPitch2EnumOptions.append("Channel2");
    FixedWingPitch2EnumOptions.append("Channel3");
    FixedWingPitch2EnumOptions.append("Channel4");
    FixedWingPitch2EnumOptions.append("Channel5");
    FixedWingPitch2EnumOptions.append("Channel6");
    FixedWingPitch2EnumOptions.append("Channel7");
    FixedWingPitch2EnumOptions.append("Channel8");
    FixedWingPitch2EnumOptions.append("None");
    fields.append( new UAVObjectField(QString("FixedWingPitch2"), QString("channel"), UAVObjectField::ENUM, FixedWingPitch2ElemNames, FixedWingPitch2EnumOptions) );
    QStringList FixedWingYawElemNames;
    FixedWingYawElemNames.append("0");
    QStringList FixedWingYawEnumOptions;
    FixedWingYawEnumOptions.append("Channel1");
    FixedWingYawEnumOptions.append("Channel2");
    FixedWingYawEnumOptions.append("Channel3");
    FixedWingYawEnumOptions.append("Channel4");
    FixedWingYawEnumOptions.append("Channel5");
    FixedWingYawEnumOptions.append("Channel6");
    FixedWingYawEnumOptions.append("Channel7");
    FixedWingYawEnumOptions.append("Channel8");
    FixedWingYawEnumOptions.append("None");
    fields.append( new UAVObjectField(QString("FixedWingYaw"), QString("channel"), UAVObjectField::ENUM, FixedWingYawElemNames, FixedWingYawEnumOptions) );
    QStringList FixedWingThrottleElemNames;
    FixedWingThrottleElemNames.append("0");
    QStringList FixedWingThrottleEnumOptions;
    FixedWingThrottleEnumOptions.append("Channel1");
    FixedWingThrottleEnumOptions.append("Channel2");
    FixedWingThrottleEnumOptions.append("Channel3");
    FixedWingThrottleEnumOptions.append("Channel4");
    FixedWingThrottleEnumOptions.append("Channel5");
    FixedWingThrottleEnumOptions.append("Channel6");
    FixedWingThrottleEnumOptions.append("Channel7");
    FixedWingThrottleEnumOptions.append("Channel8");
    FixedWingThrottleEnumOptions.append("None");
    fields.append( new UAVObjectField(QString("FixedWingThrottle"), QString("channel"), UAVObjectField::ENUM, FixedWingThrottleElemNames, FixedWingThrottleEnumOptions) );
    QStringList VTOLMotorNElemNames;
    VTOLMotorNElemNames.append("0");
    QStringList VTOLMotorNEnumOptions;
    VTOLMotorNEnumOptions.append("Channel1");
    VTOLMotorNEnumOptions.append("Channel2");
    VTOLMotorNEnumOptions.append("Channel3");
    VTOLMotorNEnumOptions.append("Channel4");
    VTOLMotorNEnumOptions.append("Channel5");
    VTOLMotorNEnumOptions.append("Channel6");
    VTOLMotorNEnumOptions.append("Channel7");
    VTOLMotorNEnumOptions.append("Channel8");
    VTOLMotorNEnumOptions.append("None");
    fields.append( new UAVObjectField(QString("VTOLMotorN"), QString("channel"), UAVObjectField::ENUM, VTOLMotorNElemNames, VTOLMotorNEnumOptions) );
    QStringList VTOLMotorNEElemNames;
    VTOLMotorNEElemNames.append("0");
    QStringList VTOLMotorNEEnumOptions;
    VTOLMotorNEEnumOptions.append("Channel1");
    VTOLMotorNEEnumOptions.append("Channel2");
    VTOLMotorNEEnumOptions.append("Channel3");
    VTOLMotorNEEnumOptions.append("Channel4");
    VTOLMotorNEEnumOptions.append("Channel5");
    VTOLMotorNEEnumOptions.append("Channel6");
    VTOLMotorNEEnumOptions.append("Channel7");
    VTOLMotorNEEnumOptions.append("Channel8");
    VTOLMotorNEEnumOptions.append("None");
    fields.append( new UAVObjectField(QString("VTOLMotorNE"), QString("channel"), UAVObjectField::ENUM, VTOLMotorNEElemNames, VTOLMotorNEEnumOptions) );
    QStringList VTOLMotorEElemNames;
    VTOLMotorEElemNames.append("0");
    QStringList VTOLMotorEEnumOptions;
    VTOLMotorEEnumOptions.append("Channel1");
    VTOLMotorEEnumOptions.append("Channel2");
    VTOLMotorEEnumOptions.append("Channel3");
    VTOLMotorEEnumOptions.append("Channel4");
    VTOLMotorEEnumOptions.append("Channel5");
    VTOLMotorEEnumOptions.append("Channel6");
    VTOLMotorEEnumOptions.append("Channel7");
    VTOLMotorEEnumOptions.append("Channel8");
    VTOLMotorEEnumOptions.append("None");
    fields.append( new UAVObjectField(QString("VTOLMotorE"), QString("channel"), UAVObjectField::ENUM, VTOLMotorEElemNames, VTOLMotorEEnumOptions) );
    QStringList VTOLMotorSEElemNames;
    VTOLMotorSEElemNames.append("0");
    QStringList VTOLMotorSEEnumOptions;
    VTOLMotorSEEnumOptions.append("Channel1");
    VTOLMotorSEEnumOptions.append("Channel2");
    VTOLMotorSEEnumOptions.append("Channel3");
    VTOLMotorSEEnumOptions.append("Channel4");
    VTOLMotorSEEnumOptions.append("Channel5");
    VTOLMotorSEEnumOptions.append("Channel6");
    VTOLMotorSEEnumOptions.append("Channel7");
    VTOLMotorSEEnumOptions.append("Channel8");
    VTOLMotorSEEnumOptions.append("None");
    fields.append( new UAVObjectField(QString("VTOLMotorSE"), QString("channel"), UAVObjectField::ENUM, VTOLMotorSEElemNames, VTOLMotorSEEnumOptions) );
    QStringList VTOLMotorSElemNames;
    VTOLMotorSElemNames.append("0");
    QStringList VTOLMotorSEnumOptions;
    VTOLMotorSEnumOptions.append("Channel1");
    VTOLMotorSEnumOptions.append("Channel2");
    VTOLMotorSEnumOptions.append("Channel3");
    VTOLMotorSEnumOptions.append("Channel4");
    VTOLMotorSEnumOptions.append("Channel5");
    VTOLMotorSEnumOptions.append("Channel6");
    VTOLMotorSEnumOptions.append("Channel7");
    VTOLMotorSEnumOptions.append("Channel8");
    VTOLMotorSEnumOptions.append("None");
    fields.append( new UAVObjectField(QString("VTOLMotorS"), QString("channel"), UAVObjectField::ENUM, VTOLMotorSElemNames, VTOLMotorSEnumOptions) );
    QStringList VTOLMotorSWElemNames;
    VTOLMotorSWElemNames.append("0");
    QStringList VTOLMotorSWEnumOptions;
    VTOLMotorSWEnumOptions.append("Channel1");
    VTOLMotorSWEnumOptions.append("Channel2");
    VTOLMotorSWEnumOptions.append("Channel3");
    VTOLMotorSWEnumOptions.append("Channel4");
    VTOLMotorSWEnumOptions.append("Channel5");
    VTOLMotorSWEnumOptions.append("Channel6");
    VTOLMotorSWEnumOptions.append("Channel7");
    VTOLMotorSWEnumOptions.append("Channel8");
    VTOLMotorSWEnumOptions.append("None");
    fields.append( new UAVObjectField(QString("VTOLMotorSW"), QString("channel"), UAVObjectField::ENUM, VTOLMotorSWElemNames, VTOLMotorSWEnumOptions) );
    QStringList VTOLMotorWElemNames;
    VTOLMotorWElemNames.append("0");
    QStringList VTOLMotorWEnumOptions;
    VTOLMotorWEnumOptions.append("Channel1");
    VTOLMotorWEnumOptions.append("Channel2");
    VTOLMotorWEnumOptions.append("Channel3");
    VTOLMotorWEnumOptions.append("Channel4");
    VTOLMotorWEnumOptions.append("Channel5");
    VTOLMotorWEnumOptions.append("Channel6");
    VTOLMotorWEnumOptions.append("Channel7");
    VTOLMotorWEnumOptions.append("Channel8");
    VTOLMotorWEnumOptions.append("None");
    fields.append( new UAVObjectField(QString("VTOLMotorW"), QString("channel"), UAVObjectField::ENUM, VTOLMotorWElemNames, VTOLMotorWEnumOptions) );
    QStringList VTOLMotorNWElemNames;
    VTOLMotorNWElemNames.append("0");
    QStringList VTOLMotorNWEnumOptions;
    VTOLMotorNWEnumOptions.append("Channel1");
    VTOLMotorNWEnumOptions.append("Channel2");
    VTOLMotorNWEnumOptions.append("Channel3");
    VTOLMotorNWEnumOptions.append("Channel4");
    VTOLMotorNWEnumOptions.append("Channel5");
    VTOLMotorNWEnumOptions.append("Channel6");
    VTOLMotorNWEnumOptions.append("Channel7");
    VTOLMotorNWEnumOptions.append("Channel8");
    VTOLMotorNWEnumOptions.append("None");
    fields.append( new UAVObjectField(QString("VTOLMotorNW"), QString("channel"), UAVObjectField::ENUM, VTOLMotorNWElemNames, VTOLMotorNWEnumOptions) );
    QStringList ChannelUpdateFreqElemNames;
    ChannelUpdateFreqElemNames.append("0");
    ChannelUpdateFreqElemNames.append("1");
    fields.append( new UAVObjectField(QString("ChannelUpdateFreq"), QString("Hz"), UAVObjectField::INT16, ChannelUpdateFreqElemNames, QStringList()) );
    QStringList ChannelMaxElemNames;
    ChannelMaxElemNames.append("0");
    ChannelMaxElemNames.append("1");
    ChannelMaxElemNames.append("2");
    ChannelMaxElemNames.append("3");
    ChannelMaxElemNames.append("4");
    ChannelMaxElemNames.append("5");
    ChannelMaxElemNames.append("6");
    ChannelMaxElemNames.append("7");
    fields.append( new UAVObjectField(QString("ChannelMax"), QString("us"), UAVObjectField::INT16, ChannelMaxElemNames, QStringList()) );
    QStringList ChannelNeutralElemNames;
    ChannelNeutralElemNames.append("0");
    ChannelNeutralElemNames.append("1");
    ChannelNeutralElemNames.append("2");
    ChannelNeutralElemNames.append("3");
    ChannelNeutralElemNames.append("4");
    ChannelNeutralElemNames.append("5");
    ChannelNeutralElemNames.append("6");
    ChannelNeutralElemNames.append("7");
    fields.append( new UAVObjectField(QString("ChannelNeutral"), QString("us"), UAVObjectField::INT16, ChannelNeutralElemNames, QStringList()) );
    QStringList ChannelMinElemNames;
    ChannelMinElemNames.append("0");
    ChannelMinElemNames.append("1");
    ChannelMinElemNames.append("2");
    ChannelMinElemNames.append("3");
    ChannelMinElemNames.append("4");
    ChannelMinElemNames.append("5");
    ChannelMinElemNames.append("6");
    ChannelMinElemNames.append("7");
    fields.append( new UAVObjectField(QString("ChannelMin"), QString("us"), UAVObjectField::INT16, ChannelMinElemNames, QStringList()) );

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
UAVObject::Metadata ActuatorSettings::getDefaultMetadata()
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
void ActuatorSettings::setDefaultFieldValues()
{
    data.FixedWingRoll1 = 8;
    data.FixedWingRoll2 = 8;
    data.FixedWingPitch1 = 8;
    data.FixedWingPitch2 = 8;
    data.FixedWingYaw = 8;
    data.FixedWingThrottle = 8;
    data.VTOLMotorN = 8;
    data.VTOLMotorNE = 8;
    data.VTOLMotorE = 8;
    data.VTOLMotorSE = 8;
    data.VTOLMotorS = 8;
    data.VTOLMotorSW = 8;
    data.VTOLMotorW = 8;
    data.VTOLMotorNW = 8;
    data.ChannelUpdateFreq[0] = 50;
    data.ChannelUpdateFreq[1] = 50;
    data.ChannelMax[0] = 1000;
    data.ChannelMax[1] = 1000;
    data.ChannelMax[2] = 1000;
    data.ChannelMax[3] = 1000;
    data.ChannelMax[4] = 1000;
    data.ChannelMax[5] = 1000;
    data.ChannelMax[6] = 1000;
    data.ChannelMax[7] = 1000;
    data.ChannelNeutral[0] = 1000;
    data.ChannelNeutral[1] = 1000;
    data.ChannelNeutral[2] = 1000;
    data.ChannelNeutral[3] = 1000;
    data.ChannelNeutral[4] = 1000;
    data.ChannelNeutral[5] = 1000;
    data.ChannelNeutral[6] = 1000;
    data.ChannelNeutral[7] = 1000;
    data.ChannelMin[0] = 1000;
    data.ChannelMin[1] = 1000;
    data.ChannelMin[2] = 1000;
    data.ChannelMin[3] = 1000;
    data.ChannelMin[4] = 1000;
    data.ChannelMin[5] = 1000;
    data.ChannelMin[6] = 1000;
    data.ChannelMin[7] = 1000;

}

/**
 * Get the object data fields
 */
ActuatorSettings::DataFields ActuatorSettings::getData()
{
    QMutexLocker locker(mutex);
    return data;
}

/**
 * Set the object data fields
 */
void ActuatorSettings::setData(const DataFields& data)
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
UAVDataObject* ActuatorSettings::clone(quint32 instID)
{
    ActuatorSettings* obj = new ActuatorSettings();
    obj->initialize(instID, this->getMetaObject());
    return obj;
}

/**
 * Static function to retrieve an instance of the object.
 */
ActuatorSettings* ActuatorSettings::GetInstance(UAVObjectManager* objMngr, quint32 instID)
{
    return dynamic_cast<ActuatorSettings*>(objMngr->getObject(ActuatorSettings::OBJID, instID));
}
