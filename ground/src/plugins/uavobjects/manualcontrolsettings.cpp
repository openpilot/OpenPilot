/**
 ******************************************************************************
 *
 * @file       manualcontrolsettings.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectsPlugin UAVObjects Plugin
 * @{
 *   
 * @note       Object definition file: manualcontrolsettings.xml. 
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
#include "manualcontrolsettings.h"
#include "uavobjectfield.h"

const QString ManualControlSettings::NAME = QString("ManualControlSettings");

/**
 * Constructor
 */
ManualControlSettings::ManualControlSettings(): UAVDataObject(OBJID, ISSINGLEINST, ISSETTINGS, NAME)
{
    // Create fields
    QList<UAVObjectField*> fields;
    QStringList InputModeElemNames;
    InputModeElemNames.append("0");
    QStringList InputModeEnumOptions;
    InputModeEnumOptions.append("PWM");
    InputModeEnumOptions.append("PPM");
    InputModeEnumOptions.append("Spektrum");
    fields.append( new UAVObjectField(QString("InputMode"), QString(""), UAVObjectField::ENUM, InputModeElemNames, InputModeEnumOptions) );
    QStringList RollElemNames;
    RollElemNames.append("0");
    QStringList RollEnumOptions;
    RollEnumOptions.append("Channel0");
    RollEnumOptions.append("Channel1");
    RollEnumOptions.append("Channel2");
    RollEnumOptions.append("Channel3");
    RollEnumOptions.append("Channel4");
    RollEnumOptions.append("Channel5");
    RollEnumOptions.append("Channel6");
    RollEnumOptions.append("Channel7");
    RollEnumOptions.append("None");
    fields.append( new UAVObjectField(QString("Roll"), QString("channel"), UAVObjectField::ENUM, RollElemNames, RollEnumOptions) );
    QStringList PitchElemNames;
    PitchElemNames.append("0");
    QStringList PitchEnumOptions;
    PitchEnumOptions.append("Channel0");
    PitchEnumOptions.append("Channel1");
    PitchEnumOptions.append("Channel2");
    PitchEnumOptions.append("Channel3");
    PitchEnumOptions.append("Channel4");
    PitchEnumOptions.append("Channel5");
    PitchEnumOptions.append("Channel6");
    PitchEnumOptions.append("Channel7");
    PitchEnumOptions.append("None");
    fields.append( new UAVObjectField(QString("Pitch"), QString("channel"), UAVObjectField::ENUM, PitchElemNames, PitchEnumOptions) );
    QStringList YawElemNames;
    YawElemNames.append("0");
    QStringList YawEnumOptions;
    YawEnumOptions.append("Channel0");
    YawEnumOptions.append("Channel1");
    YawEnumOptions.append("Channel2");
    YawEnumOptions.append("Channel3");
    YawEnumOptions.append("Channel4");
    YawEnumOptions.append("Channel5");
    YawEnumOptions.append("Channel6");
    YawEnumOptions.append("Channel7");
    YawEnumOptions.append("None");
    fields.append( new UAVObjectField(QString("Yaw"), QString("channel"), UAVObjectField::ENUM, YawElemNames, YawEnumOptions) );
    QStringList ThrottleElemNames;
    ThrottleElemNames.append("0");
    QStringList ThrottleEnumOptions;
    ThrottleEnumOptions.append("Channel0");
    ThrottleEnumOptions.append("Channel1");
    ThrottleEnumOptions.append("Channel2");
    ThrottleEnumOptions.append("Channel3");
    ThrottleEnumOptions.append("Channel4");
    ThrottleEnumOptions.append("Channel5");
    ThrottleEnumOptions.append("Channel6");
    ThrottleEnumOptions.append("Channel7");
    ThrottleEnumOptions.append("None");
    fields.append( new UAVObjectField(QString("Throttle"), QString("channel"), UAVObjectField::ENUM, ThrottleElemNames, ThrottleEnumOptions) );
    QStringList FlightModeElemNames;
    FlightModeElemNames.append("0");
    QStringList FlightModeEnumOptions;
    FlightModeEnumOptions.append("Channel0");
    FlightModeEnumOptions.append("Channel1");
    FlightModeEnumOptions.append("Channel2");
    FlightModeEnumOptions.append("Channel3");
    FlightModeEnumOptions.append("Channel4");
    FlightModeEnumOptions.append("Channel5");
    FlightModeEnumOptions.append("Channel6");
    FlightModeEnumOptions.append("Channel7");
    FlightModeEnumOptions.append("None");
    fields.append( new UAVObjectField(QString("FlightMode"), QString("channel"), UAVObjectField::ENUM, FlightModeElemNames, FlightModeEnumOptions) );
    QStringList Accessory1ElemNames;
    Accessory1ElemNames.append("0");
    QStringList Accessory1EnumOptions;
    Accessory1EnumOptions.append("Channel0");
    Accessory1EnumOptions.append("Channel1");
    Accessory1EnumOptions.append("Channel2");
    Accessory1EnumOptions.append("Channel3");
    Accessory1EnumOptions.append("Channel4");
    Accessory1EnumOptions.append("Channel5");
    Accessory1EnumOptions.append("Channel6");
    Accessory1EnumOptions.append("Channel7");
    Accessory1EnumOptions.append("None");
    fields.append( new UAVObjectField(QString("Accessory1"), QString("channel"), UAVObjectField::ENUM, Accessory1ElemNames, Accessory1EnumOptions) );
    QStringList Accessory2ElemNames;
    Accessory2ElemNames.append("0");
    QStringList Accessory2EnumOptions;
    Accessory2EnumOptions.append("Channel0");
    Accessory2EnumOptions.append("Channel1");
    Accessory2EnumOptions.append("Channel2");
    Accessory2EnumOptions.append("Channel3");
    Accessory2EnumOptions.append("Channel4");
    Accessory2EnumOptions.append("Channel5");
    Accessory2EnumOptions.append("Channel6");
    Accessory2EnumOptions.append("Channel7");
    Accessory2EnumOptions.append("None");
    fields.append( new UAVObjectField(QString("Accessory2"), QString("channel"), UAVObjectField::ENUM, Accessory2ElemNames, Accessory2EnumOptions) );
    QStringList Accessory3ElemNames;
    Accessory3ElemNames.append("0");
    QStringList Accessory3EnumOptions;
    Accessory3EnumOptions.append("Channel0");
    Accessory3EnumOptions.append("Channel1");
    Accessory3EnumOptions.append("Channel2");
    Accessory3EnumOptions.append("Channel3");
    Accessory3EnumOptions.append("Channel4");
    Accessory3EnumOptions.append("Channel5");
    Accessory3EnumOptions.append("Channel6");
    Accessory3EnumOptions.append("Channel7");
    Accessory3EnumOptions.append("None");
    fields.append( new UAVObjectField(QString("Accessory3"), QString("channel"), UAVObjectField::ENUM, Accessory3ElemNames, Accessory3EnumOptions) );
    QStringList Pos1StabilizationSettingsElemNames;
    Pos1StabilizationSettingsElemNames.append("Roll");
    Pos1StabilizationSettingsElemNames.append("Pitch");
    Pos1StabilizationSettingsElemNames.append("Yaw");
    QStringList Pos1StabilizationSettingsEnumOptions;
    Pos1StabilizationSettingsEnumOptions.append("None");
    Pos1StabilizationSettingsEnumOptions.append("Rate");
    Pos1StabilizationSettingsEnumOptions.append("Attitude");
    fields.append( new UAVObjectField(QString("Pos1StabilizationSettings"), QString(""), UAVObjectField::ENUM, Pos1StabilizationSettingsElemNames, Pos1StabilizationSettingsEnumOptions) );
    QStringList Pos2StabilizationSettingsElemNames;
    Pos2StabilizationSettingsElemNames.append("Roll");
    Pos2StabilizationSettingsElemNames.append("Pitch");
    Pos2StabilizationSettingsElemNames.append("Yaw");
    QStringList Pos2StabilizationSettingsEnumOptions;
    Pos2StabilizationSettingsEnumOptions.append("None");
    Pos2StabilizationSettingsEnumOptions.append("Rate");
    Pos2StabilizationSettingsEnumOptions.append("Attitude");
    fields.append( new UAVObjectField(QString("Pos2StabilizationSettings"), QString(""), UAVObjectField::ENUM, Pos2StabilizationSettingsElemNames, Pos2StabilizationSettingsEnumOptions) );
    QStringList Pos3StabilizationSettingsElemNames;
    Pos3StabilizationSettingsElemNames.append("Roll");
    Pos3StabilizationSettingsElemNames.append("Pitch");
    Pos3StabilizationSettingsElemNames.append("Yaw");
    QStringList Pos3StabilizationSettingsEnumOptions;
    Pos3StabilizationSettingsEnumOptions.append("None");
    Pos3StabilizationSettingsEnumOptions.append("Rate");
    Pos3StabilizationSettingsEnumOptions.append("Attitude");
    fields.append( new UAVObjectField(QString("Pos3StabilizationSettings"), QString(""), UAVObjectField::ENUM, Pos3StabilizationSettingsElemNames, Pos3StabilizationSettingsEnumOptions) );
    QStringList Pos1FlightModeElemNames;
    Pos1FlightModeElemNames.append("0");
    QStringList Pos1FlightModeEnumOptions;
    Pos1FlightModeEnumOptions.append("Manual");
    Pos1FlightModeEnumOptions.append("Stabilized");
    Pos1FlightModeEnumOptions.append("Auto");
    fields.append( new UAVObjectField(QString("Pos1FlightMode"), QString(""), UAVObjectField::ENUM, Pos1FlightModeElemNames, Pos1FlightModeEnumOptions) );
    QStringList Pos2FlightModeElemNames;
    Pos2FlightModeElemNames.append("0");
    QStringList Pos2FlightModeEnumOptions;
    Pos2FlightModeEnumOptions.append("Manual");
    Pos2FlightModeEnumOptions.append("Stabilized");
    Pos2FlightModeEnumOptions.append("Auto");
    fields.append( new UAVObjectField(QString("Pos2FlightMode"), QString(""), UAVObjectField::ENUM, Pos2FlightModeElemNames, Pos2FlightModeEnumOptions) );
    QStringList Pos3FlightModeElemNames;
    Pos3FlightModeElemNames.append("0");
    QStringList Pos3FlightModeEnumOptions;
    Pos3FlightModeEnumOptions.append("Manual");
    Pos3FlightModeEnumOptions.append("Stabilized");
    Pos3FlightModeEnumOptions.append("Auto");
    fields.append( new UAVObjectField(QString("Pos3FlightMode"), QString(""), UAVObjectField::ENUM, Pos3FlightModeElemNames, Pos3FlightModeEnumOptions) );
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
}

/**
 * Get the default metadata for this object
 */
UAVObject::Metadata ManualControlSettings::getDefaultMetadata()
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
void ManualControlSettings::setDefaultFieldValues()
{
    data.InputMode = 0;
    data.Roll = 0;
    data.Pitch = 1;
    data.Yaw = 2;
    data.Throttle = 3;
    data.FlightMode = 4;
    data.Accessory1 = 8;
    data.Accessory2 = 8;
    data.Accessory3 = 8;
    data.Pos1StabilizationSettings[0] = 2;
    data.Pos1StabilizationSettings[1] = 2;
    data.Pos1StabilizationSettings[2] = 2;
    data.Pos2StabilizationSettings[0] = 2;
    data.Pos2StabilizationSettings[1] = 2;
    data.Pos2StabilizationSettings[2] = 2;
    data.Pos3StabilizationSettings[0] = 2;
    data.Pos3StabilizationSettings[1] = 2;
    data.Pos3StabilizationSettings[2] = 2;
    data.Pos1FlightMode = 0;
    data.Pos2FlightMode = 1;
    data.Pos3FlightMode = 2;
    data.ChannelMax[0] = 2000;
    data.ChannelMax[1] = 2000;
    data.ChannelMax[2] = 2000;
    data.ChannelMax[3] = 2000;
    data.ChannelMax[4] = 2000;
    data.ChannelMax[5] = 2000;
    data.ChannelMax[6] = 2000;
    data.ChannelMax[7] = 2000;
    data.ChannelNeutral[0] = 1500;
    data.ChannelNeutral[1] = 1500;
    data.ChannelNeutral[2] = 1500;
    data.ChannelNeutral[3] = 1500;
    data.ChannelNeutral[4] = 1500;
    data.ChannelNeutral[5] = 1500;
    data.ChannelNeutral[6] = 1500;
    data.ChannelNeutral[7] = 1500;
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
ManualControlSettings::DataFields ManualControlSettings::getData()
{
    QMutexLocker locker(mutex);
    return data;
}

/**
 * Set the object data fields
 */
void ManualControlSettings::setData(const DataFields& data)
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
UAVDataObject* ManualControlSettings::clone(quint32 instID)
{
    ManualControlSettings* obj = new ManualControlSettings();
    obj->initialize(instID, this->getMetaObject());
    return obj;
}

/**
 * Static function to retrieve an instance of the object.
 */
ManualControlSettings* ManualControlSettings::GetInstance(UAVObjectManager* objMngr, quint32 instID)
{
    return dynamic_cast<ManualControlSettings*>(objMngr->getObject(ManualControlSettings::OBJID, instID));
}
